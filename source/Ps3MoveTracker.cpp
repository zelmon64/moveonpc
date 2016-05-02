/**
 * \brief  Demo for basic PlayStation Move usage (header file)
 * \author Alexander Nitsch <nitscha@cs.tu-berlin.de>
 */

#include "Ps3MoveTracker.h"


bool Ps3MoveTracker::start() {
	// create window to show the camera image
	cvNamedWindow(m_pszWindowName, CV_WINDOW_AUTOSIZE);

	// create sliders for camera parameters
	int v = 0;
	cvCreateTrackbar2("exposure", m_pszWindowName, &v, 511, &Ps3MoveTracker::sliderExposureProc, this);	
	cvCreateTrackbar2("gain",     m_pszWindowName, &v,  79, &Ps3MoveTracker::sliderGainProc,     this);

	// create sliders for image processing parameters
	m_iRemoveNoise = 1;
	m_iFloodFill = 51;
	cvCreateTrackbar("min hue",    m_pszWindowName, &m_iMinHue,      180, NULL);
	cvCreateTrackbar("max hue",    m_pszWindowName, &m_iMaxHue,      180, NULL);
	cvCreateTrackbar("rm noise",   m_pszWindowName, &m_iRemoveNoise,   1, NULL);
	cvCreateTrackbar("flood fill", m_pszWindowName, &m_iFloodFill,    90, NULL);

	// create periodic 12ms-timer to generate clock ticks for sensor thread
	// (needed to prevent flooding the controller with thousands of requests
	// per second)
	m_hSensorTimer = CreateWaitableTimer(NULL, FALSE, NULL);
	if (m_hSensorTimer == NULL) {
		printf("Failed to create timer for sensor thread (error %d).\n", GetLastError());
		return false;
	}
	LARGE_INTEGER liDueTime; liDueTime.QuadPart = 0;
	if (!SetWaitableTimer(m_hSensorTimer, &liDueTime, 12, NULL, NULL, FALSE)) {
		stop();
		printf("Failed to set timer for sensor thread (error %d).\n", GetLastError());
		return false;
	}

	// init critical section used by sensor and camera thread
	InitializeCriticalSection(&m_criticalSection);

	// create thread that will exchange data with the Move controller
	m_bSensorRunning = true;
	m_hSensorThread = CreateThread(NULL, 0, &Ps3MoveTracker::sensorThreadProc, this, 0, NULL);
	if (m_hSensorThread == NULL) {
		stop();
		printf("Failed to create sensor worker thread.\n");
		return false;
	}

	// create thread that will capture images from the camera
	m_hCameraRunning = true;
	m_hCameraThread = CreateThread(NULL, 0, &Ps3MoveTracker::cameraThreadProc, this, 0, NULL);
	if (m_hCameraThread == NULL) {
		stop();
		printf("Failed to create camera worker thread.\n");
		return false;
	}

	return true;
}


bool Ps3MoveTracker::stop() {
	if (m_bSensorRunning) {
		m_bSensorRunning = false;
		// wait for thread to exit, timeout after 2 seconds
		WaitForSingleObject(m_hSensorThread, 2000);

		// report the return value of sensorThreadProc()
		//DWORD dwExitCode;
		//GetExitCodeThread(m_hSensorThread, &dwExitCode);

		CloseHandle(m_hSensorThread);
		CloseHandle(m_hSensorTimer);
	}

	if (m_hCameraRunning) {
		m_hCameraRunning = false;
		// wait for thread to exit, timeout after 2 seconds
		WaitForSingleObject(m_hCameraThread, 2000);
		CloseHandle(m_hCameraThread);
	}

	DeleteCriticalSection(&m_criticalSection);

	cvDestroyWindow(m_pszWindowName);

	return true;
}


void Ps3MoveTracker::runSensor() {
	TMove moveData, moveDataOld;
	struct {
		unsigned int r;
		unsigned int g;
		unsigned int b;
	} moveColor = { 0, 0, 0 };
	IirFilter *pcLowPassFilterAcc[3];
	IirFilter *pcLowPassFilterMag[3];

	double a[3], m[3];

	// rotation matrix
	double R[3][3];

	// reference matrix, describing the unit vectors of the reference system
	// set in the calibration step (initially the global system, i.e. simply
	// the identity matrix)
	double Rref[3][3] = { {1, 0, 0}, {0, 1, 0}, {0, 0, 1} };

	// calibration data for normalisation (only rough estimates)
	// magnetometer data needs to be adjusted for new locations!
	CalibrationData cal;
	if (!readCalibrationData("../config/calibration.yaml", &cal)) {
		printf("Failed to read controller's calibration data.\n");
		return;
	}

	if (!OpenMoves() || (GetMoveCount() < 1)) {
		printf("Failed to open Move controller.\n");
		return;
	}

	// TODO: In error cases, instead of just returning, signal that there was a
	//       problem so that other parts of the program, relying on this thread,
	//       can react accordingly.


	// contruct simple 1-pole low-pass filters for sensor data
	double k = 0.18;
	double coeffs[2] = { 1, -(1-k) };
	for (int i = 0; i < 3; i++) {
		pcLowPassFilterAcc[i] = new IirFilter(&k, 1, coeffs, 2);
		pcLowPassFilterMag[i] = new IirFilter(&k, 1, coeffs, 2);
	}

	while (m_bSensorRunning) {
		// read sensor data from controller
		ReadMove(0, &moveData, &moveDataOld);

		// set Move's color depending on the buttons pressed
		if (moveData.Buttons) {
			if (moveData.Buttons & B_CIRCLE) {
				moveColor.r = moveData.TAnalog;
			}
			if (moveData.Buttons & B_TRIANGLE) {
				moveColor.g = moveData.TAnalog;
			}
			if (moveData.Buttons & B_CROSS) {
				moveColor.b = moveData.TAnalog;
			}
		}
		SetMoveColour(0, moveColor.r, moveColor.g, moveColor.b);

		// normalise accelerometer and magnetometer data to [-1, 1]
		// (swap Z and Y axes in the process because I want a right-hand system
		// with Z pointing up ...)
		a[0] = (2*moveData.RawForceX  - (cal.acc[0][1] + cal.acc[0][0])) / (cal.acc[0][1] - cal.acc[0][0]);
		a[1] = (2*moveData.RawForceZ  - (cal.acc[1][1] + cal.acc[1][0])) / (cal.acc[1][1] - cal.acc[1][0]);
		a[2] = (2*moveData.RawForceY  - (cal.acc[2][1] + cal.acc[2][0])) / (cal.acc[2][1] - cal.acc[2][0]);
		m[0] = (2*moveData.RawMagnetX - (cal.mag[0][1] + cal.mag[0][0])) / (cal.mag[0][1] - cal.mag[0][0]);
		m[1] = (2*moveData.RawMagnetZ - (cal.mag[1][1] + cal.mag[1][0])) / (cal.mag[1][1] - cal.mag[1][0]);
		m[2] = (2*moveData.RawMagnetY - (cal.mag[2][1] + cal.mag[2][0])) / (cal.mag[2][1] - cal.mag[2][0]);

		// apply low-pass filter to sensor data
		for (int i = 0; i < 3; i++) {
			a[i] = pcLowPassFilterAcc[i]->step(a[i]);
			m[i] = pcLowPassFilterMag[i]->step(m[i]);
		}

		// get Z axis from normalised accelerometer data (gravity)
		R[2][0] = a[0];
		R[2][1] = a[1];
		R[2][2] = a[2];
		normalise(R[2]);

		// calculate EAST axis from Z axis and normalised magnetometer data
		R[0][0] = -m[0];
		R[0][1] = -m[1];
		R[0][2] = -m[2];
		cross(R[2], R[0], R[0]);
		normalise(R[0]);

		// calculate NORTH axis from Z axis and EAST axis
		cross(R[2], R[0], R[1]);

		// calibrate if Move button is pressed
		m_bCalibrating = false;
		if (moveData.Buttons & B_MOVE) {
			m_bCalibrating = true;

			// update reference system with values from transposed current
			// rotation matrix, i.e. the current rotated system is used as the
			// new reference from now on
			for (int i = 0; i < 3; i++) {
				for (int j = 0; j < 3; j++) {
					Rref[i][j] = R[j][i];
				}
			}
		}

		// change rotation matrix to perform the rotation about the reference
		// axes instead of the globally fixed axes from absolute orientation
		EnterCriticalSection(&m_criticalSection);
		matrixMult(Rref, R, m_pdRotMatrix);
		LeaveCriticalSection(&m_criticalSection);

		// wait for next clock tick, timeout after 1 second
		WaitForSingleObject(m_hSensorTimer, 1000);

		// TODO: Report timeouts or other problems with this thread's clock.

		// TODO: Replace the above calculation of the absolute orientation with
		//       something more sophisticated. The current algorithm doesn't
		//       even use the gyros. Due to low-pass filtering (to get rid of
		//       the noise, especially in the magnetometer data) the detection
		//       responds only slowly to fast movement of the controller.
	}

	CloseMoves();

	for (int i = 0; i < 3; i++) {
		delete pcLowPassFilterAcc[i];
		delete pcLowPassFilterMag[i];
	}
}


void Ps3MoveTracker::runCamera() {
	int imgWidth, imgHeight;
	IplImage *pImg;
	PBYTE pImgBuf;
	DWORD dwTimeStart, dwTimeStop;
	int n = 0;
	CircleTracker *pcCircleTracker;
	CircleParams circle;
	double R[3][3]    = { { 1, 0, 0}, {0,  1, 0}, {0, 0, 1} };
	double Rcam[3][3] = { {-1, 0, 0}, {0, -1, 0}, {0, 0, 1} };


	m_camera = CLEyeCreateCamera(m_cameraGuid, m_eCameraColorMode, m_eCameraResolution, m_fFrameRate);
	if (m_camera == NULL) {
		return;

		// TODO: Signal that there was a problem so that other parts of the
		//       program, relying on this thread, can react accordingly.
	}

	CLEyeCameraGetFrameDimensions(m_camera, imgWidth, imgHeight);

	// create image header and allocate image data
	pImg = cvCreateImage(cvSize(imgWidth, imgHeight), IPL_DEPTH_8U, 4);
	cvGetRawData(pImg, &pImgBuf);

	// initialize parameter sliders
	CLEyeSetCameraParameter(m_camera, CLEYE_EXPOSURE, 310);
	CLEyeSetCameraParameter(m_camera, CLEYE_GAIN, 10);
	cvSetTrackbarPos("exposure", m_pszWindowName, CLEyeGetCameraParameter(m_camera, CLEYE_EXPOSURE));
	cvSetTrackbarPos("gain",     m_pszWindowName, CLEyeGetCameraParameter(m_camera, CLEYE_GAIN));

	CLEyeCameraStart(m_camera);

	// create the tracker that will find the glowing ball in the frames
	pcCircleTracker = new CircleTracker();
	
	// activate for debugging ...
	//pcCircleTracker->setShowMask(true);
	pcCircleTracker->setShowContour(true);

	dwTimeStart = timeGetTime();

	while (m_hCameraRunning) {
		CLEyeCameraGetFrame(m_camera, pImgBuf);

		// approximate actual frame rate from time difference between
		// m_fFrameRate frames (updates roughly every second)
		if (++n >= m_fFrameRate) {
			dwTimeStop = timeGetTime();
			m_fCurFrameRate = m_fFrameRate * 1000.0f / (dwTimeStop - dwTimeStart);
			dwTimeStart = dwTimeStop;
			n = 0;
		}

		// find the glowing ball in the frame
		if (!pcCircleTracker->trackCircle(&circle, pImg, m_iMinHue, m_iMaxHue,
			m_iRemoveNoise > 0, m_iFloodFill))
		{
			//printf("No circle found in image.\n");
		} else {
			// DEBUG: output circle's center coordinates and radius
			//printf("\tcx: %7.3f, cy: %7.3f, r: %f\n", circle.x, circle.y, circle.r);

			// DEBUG: Draw circle and its center point as our guess for ball's
			//        location and size (coordinates and radius rounded to
			//        integer pixels).
			int x = (int)(0.5 + circle.x);
			int y = (int)(0.5 + circle.y);
			int r = (int)(0.5 + circle.r);
			int d = 5;
			CvScalar color = CV_RGB(255,255,0);
			cvCircle(pImg, cvPoint(x, y), r, color, 2);
			cvLine(pImg, cvPoint(x-d, y), cvPoint(x+d, y), color);
			cvLine(pImg, cvPoint(x, y-d), cvPoint(x, y+d), color);

			// read rotation matrix (set by sensor thread)
			EnterCriticalSection(&m_criticalSection);
			for (int i = 0; i < 3; i++) {
				for (int j = 0; j < 3; j++) {
					R[i][j] = m_pdRotMatrix[i][j];
				}
			}
			LeaveCriticalSection(&m_criticalSection);

			// Calculate the glowing ball's position in 3D space from circle's
			// parameters and camera's intrinsic parameters.

			// camera's focal length (retrieved from calibration of the PS Eye)
			// TODO: read this value from config file instead of hard-coding it
			double f = 540;

			// ball's true radius in the scene (in mm)
			double rBall = 22.5;

			// distance between camera and ball's center in 3D space
			double Y = f * rBall / circle.r;

			// X (right) and Z (up) coordinates of ball's center in 3D space
			double X =   (circle.x - pImg->width/2)  * (Y / f);
			double Z = - (circle.y - pImg->height/2) * (Y / f);

			printf("\rdistance from camera: %8.2f cm", Y / 10.0);

			// transform controller's local coordinates into screen
			// coordinates (rotate 180 degrees about Z (UP in camera and
			// local) to make the camera look from positive end of Y-axis
			matrixMult(Rcam, R, R);


			Geom3d cPlot3d(pImg->width/2, pImg->height/2, f);

			{
				// 3D axes

				Object3d *pcAxes = Geom3d::getAxes(1.8 * rBall);
				
				// rotate according to controller's current orientiation
				pcAxes->rotate(R);
				
				// translate to ball's current center point
				pcAxes->translate(X, Y, Z);

				CvScalar colorR = CV_RGB(255, 0, 0);
				CvScalar colorG = CV_RGB(0, 255, 0);
				CvScalar colorB = CV_RGB(0, 0, 255);

				// draw in correct depth order
				// (Note that this is quite hackish. But since it's not supposed
				// to replace a full-fledged 3D renderer ...)
				if ((R[1][0] <= R[1][1]) && (R[1][0] <= R[1][2])) {
					if (R[1][1] <= R[1][2]) {
						cPlot3d.plotLine(pImg, pcAxes->m_cVertices[0], pcAxes->m_cVertices[3], colorB, 2);
						cPlot3d.plotLine(pImg, pcAxes->m_cVertices[0], pcAxes->m_cVertices[2], colorG, 2);
					} else  {
						cPlot3d.plotLine(pImg, pcAxes->m_cVertices[0], pcAxes->m_cVertices[2], colorG, 2);
						cPlot3d.plotLine(pImg, pcAxes->m_cVertices[0], pcAxes->m_cVertices[3], colorB, 2);
					}
					cPlot3d.plotLine(pImg, pcAxes->m_cVertices[0], pcAxes->m_cVertices[1], colorR, 2);
				} else if ((R[1][1] <= R[1][0]) && (R[1][1] <= R[1][2])) {
					if (R[1][0] <= R[1][2]) {
						cPlot3d.plotLine(pImg, pcAxes->m_cVertices[0], pcAxes->m_cVertices[3], colorB, 2);
						cPlot3d.plotLine(pImg, pcAxes->m_cVertices[0], pcAxes->m_cVertices[1], colorR, 2);
					} else {
						cPlot3d.plotLine(pImg, pcAxes->m_cVertices[0], pcAxes->m_cVertices[1], colorR, 2);
						cPlot3d.plotLine(pImg, pcAxes->m_cVertices[0], pcAxes->m_cVertices[3], colorB, 2);
					}			
					cPlot3d.plotLine(pImg, pcAxes->m_cVertices[0], pcAxes->m_cVertices[2], colorG, 2);
				} else {
					if (R[1][0] <= R[1][1]) {
						cPlot3d.plotLine(pImg, pcAxes->m_cVertices[0], pcAxes->m_cVertices[2], colorG, 2);
						cPlot3d.plotLine(pImg, pcAxes->m_cVertices[0], pcAxes->m_cVertices[1], colorR, 2);
					} else {
						cPlot3d.plotLine(pImg, pcAxes->m_cVertices[0], pcAxes->m_cVertices[1], colorR, 2);
						cPlot3d.plotLine(pImg, pcAxes->m_cVertices[0], pcAxes->m_cVertices[2], colorG, 2);
					}
					cPlot3d.plotLine(pImg, pcAxes->m_cVertices[0], pcAxes->m_cVertices[3], colorB, 2);
				}

				delete pcAxes;
			}

			{
				// 3D box

				Object3d *pcBox = Geom3d::getBox(40, 20, 200);
				
				// translate box to line up with controller
				pcBox->translate(0, -175, 0);
				
				// rotate according to controller's current orientiation
				pcBox->rotate(R);
				
				// translate to ball's current center point
				pcBox->translate(X, Y, Z);
				
				cPlot3d.plotObject(pImg, pcBox);

				delete pcBox;
			}
		}

		// TODO: Implement filter to predict circle's position if the tracker
		//       didn't find anything in this frame.


		// flip the image left-right to create the "look into a mirror"
		//cvFlip(pImg, pImg, 1);

		// write statistics as text onto the image
		addTextStats(pImg);

		cvShowImage(m_pszWindowName, pImg);
	}

	delete pcCircleTracker;

	CLEyeCameraStop(m_camera);
	CLEyeDestroyCamera(m_camera);

	cvReleaseImage(&pImg);
}


void Ps3MoveTracker::addTextStats(IplImage *pImg) {
	CvFont font;
	static char s[256];

	cvInitFont(&font, CV_FONT_HERSHEY_PLAIN, 1, 1);

	_snprintf_s(s, 256, 255, "frame rate: %.2f", m_fCurFrameRate);
	cvPutText(pImg, s, cvPoint(5, 15), &font, CV_RGB(255, 255, 255));

	cvPutText(pImg, "Press Move button to calibrate.", cvPoint(5, 30), &font,
		m_bCalibrating ? CV_RGB(80, 80, 80) : CV_RGB(255, 255, 255));
}


bool Ps3MoveTracker::readCalibrationData(const char *pszFilename, CalibrationData *psCalData) {
	cv::FileStorage fs(pszFilename, cv::FileStorage::READ);

	if (!fs.isOpened()) {
		return false;
	}

	cv::FileNode fn = fs["acc"];
	psCalData->acc[0][0] = (double)fn["xmin"];
	psCalData->acc[0][1] = (double)fn["xmax"];
	psCalData->acc[1][0] = (double)fn["ymin"];
	psCalData->acc[1][1] = (double)fn["ymax"];
	psCalData->acc[2][0] = (double)fn["zmin"];
	psCalData->acc[2][1] = (double)fn["zmax"];

	fn = fs["mag"];
	psCalData->mag[0][0] = (double)fn["xmin"];
	psCalData->mag[0][1] = (double)fn["xmax"];
	psCalData->mag[1][0] = (double)fn["ymin"];
	psCalData->mag[1][1] = (double)fn["ymax"];
	psCalData->mag[2][0] = (double)fn["zmin"];
	psCalData->mag[2][1] = (double)fn["zmax"];

	return true;
}


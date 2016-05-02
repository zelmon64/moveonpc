/**
 * \brief  Demo for basic PlayStation Move usage (header file)
 * \author Alexander Nitsch <nitscha@cs.tu-berlin.de>
 *
 * This class implements a tracker for the PlayStation Move controller. The
 * colored sphere is continously detected in camera images coming from the
 * PlayStation Eye camera, and thus the controller's position in 3D space is
 * known. This is combined with information about the controller's current
 * orientation, derived from its internal sensors.
 *
 * Camera and sensor are each handled in their own thread. The sensor thread
 * periodically updates the rotation matrix that represents the controller's
 * current orientation. The camera thread retrieves images from the PS Eye
 * and also handles graphical representation of tracking results.
 */

#pragma once

#include <windows.h>

#include <cv.h>
#include <highgui.h>

#include <CLEyeMulticam.h>

#include "moveonpc.h"
#include "CircleTracker.h"
#include "Matrix.h"
#include "Geom3d.h"
#include "IirFilter.h"


#define TITLE_BUFSIZE 128


struct CalibrationData {
	double acc[3][2];
	double mag[3][2];
};


class Ps3MoveTracker {
private:
	char m_pszWindowName[TITLE_BUFSIZE];

	CLEyeCameraInstance m_camera;
	GUID m_cameraGuid;
	CLEyeCameraColorMode m_eCameraColorMode;
	CLEyeCameraResolution m_eCameraResolution;

	float m_fFrameRate;
	float m_fCurFrameRate;

	bool m_hCameraRunning;
	bool m_bSensorRunning;
	bool m_bCalibrating;

	CRITICAL_SECTION m_criticalSection;
	HANDLE m_hCameraThread;
	HANDLE m_hSensorThread;
	HANDLE m_hSensorTimer;

	int m_iMinHue;
	int m_iMaxHue;
	int m_iRemoveNoise;
	int m_iFloodFill;

	double m_pdRotMatrix[3][3];

public:
	Ps3MoveTracker(const char *pszWindowName, GUID guid,
			CLEyeCameraResolution resolution, float frameRate)
	{
		strncpy_s(m_pszWindowName, TITLE_BUFSIZE, pszWindowName, TITLE_BUFSIZE);

		m_camera = NULL;
		m_cameraGuid = guid;
		m_eCameraColorMode = CLEYE_COLOR_RAW;
		m_eCameraResolution = resolution;

		m_fFrameRate = frameRate;
		m_fCurFrameRate = 0;

		m_hCameraRunning = false;
		m_bSensorRunning = false;
		m_bCalibrating = false;

		m_iMinHue =  95;
		m_iMaxHue = 100;
		m_iRemoveNoise = 0;
		m_iFloodFill = 0;

		// init rotation matrix with the identity matrix
		for (int i = 0; i < 3; i++) {
			for (int j = 0; j < 3; j++) {
				m_pdRotMatrix[i][j] = (i == j) ? 1.0 : 0.0;
			}
		}
	}

	virtual ~Ps3MoveTracker(void) {}

	bool start();
	bool stop();
	void runCamera();
	void runSensor();
	bool readCalibrationData(const char *pszFilename, CalibrationData *psCalData);

	static DWORD WINAPI cameraThreadProc(LPVOID lpParam) {
		Ps3MoveTracker *pInstance = (Ps3MoveTracker *)lpParam;
		pInstance->runCamera();
		return 0;
	}

	static DWORD WINAPI sensorThreadProc(LPVOID lpParam) {
		Ps3MoveTracker *pInstance = (Ps3MoveTracker *)lpParam;
		pInstance->runSensor();
		return 0;
	}

	static void sliderExposureProc(int value, void *instance) {
		CLEyeSetCameraParameter(((Ps3MoveTracker *)instance)->m_camera, CLEYE_EXPOSURE, value);
	}

	static void sliderGainProc(int value, void *instance) {
		CLEyeSetCameraParameter(((Ps3MoveTracker *)instance)->m_camera, CLEYE_GAIN, value);
	}

	void addTextStats(IplImage *pImg);
};

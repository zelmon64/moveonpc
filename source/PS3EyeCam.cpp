#include "PS3EyeCam.h"

void CLEyeCameraCapture::setupWindows()
{
	is_ball_detected = false;

	param1 = 0;//h min
	param2 = 0;//s min
	param3 = 160;//v min
	param4 = 256;//h max
	param5 = 256;//s max
	param6 = 256;//v max

	param7 = 0;//erode
	param8 = 0;//dilate

	gain = 0;
	exposure = 511;
	mode = 0;

	w=0; h=0;

	cvNamedWindow("calibration_window", CV_WINDOW_AUTOSIZE);
	cvNamedWindow("mask_window", CV_WINDOW_AUTOSIZE);

	cvCreateTrackbar2("minH","calibration_window",&param1,181,&CLEyeCameraCapture::switch_param1,this);		
	cvCreateTrackbar2("minS","calibration_window",&param2,256,&CLEyeCameraCapture::switch_param2,this);		
	cvCreateTrackbar2("minV","calibration_window",&param3,256,&CLEyeCameraCapture::switch_param3,this);		
	cvCreateTrackbar2("maxH","calibration_window",&param4,181,&CLEyeCameraCapture::switch_param4,this);		
	cvCreateTrackbar2("maxS","calibration_window",&param5,256,&CLEyeCameraCapture::switch_param5,this);		
	cvCreateTrackbar2("maxV","calibration_window",&param6,256,&CLEyeCameraCapture::switch_param6,this);		

	cvCreateTrackbar2("erode","calibration_window",&param7,10,&CLEyeCameraCapture::switch_param7,this);		
	cvCreateTrackbar2("dilate","calibration_window",&param8,10,&CLEyeCameraCapture::switch_param8,this);		

	cvCreateTrackbar2("Gain",_windowName,&gain,79,&CLEyeCameraCapture::switch_gain,this);		
	cvCreateTrackbar2("Exposure",_windowName,&exposure,511,&CLEyeCameraCapture::switch_exposure,this);

	cvCreateTrackbar2("mode",_windowName,&mode,3,&CLEyeCameraCapture::switch_mode,this);
}

bool CLEyeCameraCapture::StartCapture()
{
	cvNamedWindow(_windowName, CV_WINDOW_AUTOSIZE);

	////////////////////////////////////////////////
	setupWindows();
	////////////////////////////////////////////////

	_running = true;

	// Start CLEye image capture thread
	_hThread = CreateThread(NULL, 0, &CLEyeCameraCapture::CaptureThread, this, 0, 0);
	if(_hThread == NULL)
	{
		MessageBox(NULL,"Could not create capture thread","CLEyeMulticamTest", MB_ICONEXCLAMATION);
		return false;
	}
	return true;
}

void CLEyeCameraCapture::StopCapture()
{
	if(!_running)	return;
	_running = false;
	WaitForSingleObject(_hThread, 500);
	cvDestroyWindow(_windowName);

	/////////////////////////////////
	cvDestroyWindow("calibration_window");
	cvDestroyWindow("mask_window");
	/////////////////////////////////
}

void CLEyeCameraCapture::setupParams()
{
	threshold_image = cvCreateImage(cvGetSize(pCapImage),IPL_DEPTH_8U,1);
	imgHSV = cvCreateImage(cvGetSize(pCapImage), IPL_DEPTH_8U, 3);
	image_roi = Mat(w,h,CV_8UC1,Scalar(0,0,0));
	circles.resize(1); //sino peta en debug
	radius=9999;
	oldTime=0; newTime=0;
}

void CLEyeCameraCapture::colorFiltering()
{
	cvCvtColor( pCapImage, imgHSV, CV_BGR2HSV);
	min_color = Scalar(param1,param2,param3,0);
	max_color = Scalar(param4,param5,param6,0);
	cvInRangeS(imgHSV, min_color, max_color, threshold_image);
}

void CLEyeCameraCapture::opening()
{
	cvErode(threshold_image,threshold_image,0,param7);
	cvDilate(threshold_image,threshold_image,0,param8);
}

void CLEyeCameraCapture::find_ball()
{
	is_ball_detected = false;
	using_image_roi = false;
	threshold_image_mat = threshold_image;

	if(radius==9999) Hough_mat= threshold_image_mat; // no sabemos donde está la pelota, tenemos que usar toda la imagen en HoughCircles.
	else
	{
		using_image_roi=true;
		Hough_mat= image_roi;
	}

	GaussianBlur(Hough_mat, Hough_mat, Size(7,7), 1.5, 1.5);
	HoughCircles(Hough_mat, circles, CV_HOUGH_GRADIENT, 2, Hough_mat.cols, 200,100); //probar de limitar el radio entre 10 y 100 por ejemplo..
	int i;
	for( i = 0; i < circles.size(); i++ )
	{
		if(using_image_roi)
		{
			center.x = cvRound(circles[i].val[0]) + pos_rectangle_roi.x;
			center.y = cvRound(circles[i].val[1]) + pos_rectangle_roi.y;
		}
		else
		{
			center.x = (circles[i].val[0]);
			center.y = (circles[i].val[1]);
		}
		radius = cvRound(circles[i].val[2]);

		////////////// calculo del radio y el centro REAL
		rowStep = threshold_image->widthStep;
		pixel = rowStep*center.y+center.x;
		real_radiusR = real_radiusL = real_radiusT = real_radiusB = 0;
		while(threshold_image->imageData[pixel+real_radiusR])
		{
			real_radiusR++;
		}
		while(threshold_image->imageData[pixel-real_radiusL])
		{				
			real_radiusL++;
		}
		while(threshold_image->imageData[pixel-rowStep*real_radiusT])
		{
			real_radiusT++;
			if((pixel-rowStep*real_radiusT)<=0) break;
		}
		while(threshold_image->imageData[pixel+rowStep*real_radiusB])
		{
			real_radiusB++;
			if((pixel+rowStep*real_radiusB)>=w*h) break;
		}
		temp = ((real_radiusR+real_radiusL)*(real_radiusR+real_radiusL) + (real_radiusB+real_radiusT)*(real_radiusB+real_radiusT));
		newradius = ((float)sqrt(temp))/3.0;
		diffradius = abs(newradius - radius);
		////printf("diffRADIUS: %d\n",diffradius);

		newCX = (real_radiusR+real_radiusL)*0.5;
		newCY = (real_radiusB+real_radiusT)*0.5;
		if(real_radiusL < newCX) newCX = center.x + (newCX - real_radiusL);
		else newCX = center.x - (real_radiusL - newCX);
		if(real_radiusT < newCY) newCY = center.y + (newCY - real_radiusT);
		else newCY = center.y - (real_radiusT - newCY);
		diffcenter = (int)(sqrt(abs(center.x-newCX)*abs(center.x-newCX)+abs(center.y-newCY)*abs(center.y-newCY)));
		////printf("diffCENTER: %d\n",diffcenter);


		if(diffradius>10 || diffcenter>10 || mode == 2)
		{
			ball[0] = center.x;
			ball[1]= center.y;
			ball[2]= radius;
		}
		else if( mode != 2)
		{
			ball[0] = newCX;
			ball[1] = newCY;
			ball[2] = newradius;
		}

		balltoPaint[0] = (ball[0] + last_ball1[0] + last_ball2[0] + last_ball3[0])*0.25;
		balltoPaint[1] = (ball[1] + last_ball1[1] + last_ball2[1] + last_ball3[1])*0.25;
		balltoPaint[2] = (ball[2] + last_ball1[2] + last_ball2[2] + last_ball3[2])*0.25;
		is_ball_detected = true;

		//conociendo la posicion de la pelota, limitamos la region donde aplicar la busqueda por houghcircles
		offset = (w == 640)? 150:75;
		xx = cvRound(center.x-offset);
		if(xx<0) xx = 0;
		yy = cvRound(center.y-offset);
		if(yy<0) yy = 0;
		ww = cvRound(center.x+offset);
		if(ww>w) ww = w;
		hh = cvRound(center.y+offset);
		if(hh>h) hh = h;
		roi = Rect(Point(xx,yy),Point(ww,hh));
		image_roi = threshold_image_mat(roi);
		pos_rectangle_roi.x = xx;
		pos_rectangle_roi.y = yy;
		cvRectangle(pCapImage, Point(xx,yy),Point(ww,hh),Scalar(0,255,0),3);

		last_ball3 = last_ball2;
		last_ball2 = last_ball1;
		last_ball1 = ball;


		if(mode==0) // mixture of modes 2 and 3
		{
			cvRectangle(pCapImage, Point((ball[0]-ball[2]),(ball[1]-ball[2])),Point((ball[0]+ball[2]),(ball[1]+ball[2])),Scalar(0,255,0),2);
			cvLine(pCapImage,Point(pCapImage->width,ball[1]),Point(0,ball[1]),Scalar(0,255,0));
			cvLine(pCapImage,Point(ball[0],pCapImage->height),Point(ball[0],0),Scalar(0,255,0));
		}
		else if(mode==1)//linear interpolation of mode 0. 5 samples to smooth. but it ads lag
		{
			cvRectangle(pCapImage, Point((balltoPaint[0]-balltoPaint[2]),(balltoPaint[1]-balltoPaint[2])),Point((balltoPaint[0]+balltoPaint[2]),(balltoPaint[1]+balltoPaint[2])),Scalar(0,255,0),2);
			cvLine(pCapImage,Point(pCapImage->width,balltoPaint[1]),Point(0,balltoPaint[1]),Scalar(0,255,0));
			cvLine(pCapImage,Point(balltoPaint[0],pCapImage->height),Point(balltoPaint[0],0),Scalar(0,255,0));
		}
		else if(mode==2)//houghcircles. noise but works well with oclussions
		{
			cvRectangle(pCapImage, Point((center.x-radius),(center.y-radius)),Point((center.x+radius),(center.y+radius)),Scalar(0,0,255),2);
			cvLine(pCapImage,Point(pCapImage->width,center.y),Point(0,center.y),Scalar(0,0,255));
			cvLine(pCapImage,Point(center.x,pCapImage->height),Point(center.x,0),Scalar(0,0,255));
		}
		else if(mode==3)//pixel precision. but works bad with parcial oclussions
		{
			cvRectangle(pCapImage, Point((newCX-newradius),(newCY-newradius)),Point((newCX+newradius),(newCY+newradius)),Scalar(255,0,0),2);
			cvLine(pCapImage,Point(pCapImage->width,newCY),Point(0,newCY),Scalar(255,0,0));
			cvLine(pCapImage,Point(newCX,pCapImage->height),Point(newCX,0),Scalar(255,0,0));
		}

			printf("X: %f\n",ball[0]);
			printf("Y: %f\n",ball[1]);
			printf("radius: %f\n",ball[2]);
			printf("\n");

			printf("X_filtered: %f\n",balltoPaint[0]);
			printf("Y_filtered: %f\n",balltoPaint[1]);
			printf("radius_filtered: %f\n",balltoPaint[2]);
			printf("\n");
	}
	if(i==0)
	{
		//printf("miss\n");
		radius = 9999; //no se han encontrado circulos
	}

	cvFlip(threshold_image,threshold_image,1);
	cvFlip(pCapImage,pCapImage,1);

	cvShowImage("mask_window", threshold_image); //(Carl Kenner) in VS2010 debug: Unhandled exception at 0x0f5ff7e0 in Glowing_ball_trackerd.exe: 0xC0000005: Access violation reading location 0xcccccccc.
	//cvShowImage("calibration_window",0);
	imshow("calibration_window",threshold_image);
	cvShowImage(_windowName, pCapImage);
}

void CLEyeCameraCapture::destroyMyImages()
{
	cvReleaseImage(&threshold_image);
	cvReleaseImage(&imgHSV);
}

void CLEyeCameraCapture::showFPS()
{
	newTime = timeGetTime();
	float dt = newTime - oldTime;
	float fps = 1.0f / dt * 1000;
	oldTime = newTime;
	printf("fps: %0.f\n",fps);
	printf("dt: %0.f\n",dt);
	printf("\n");
}

void CLEyeCameraCapture::Run()
{
	pCapBuffer = NULL;

	// Create camera instance
	_cam = CLEyeCreateCamera(_cameraGUID, _mode, _resolution, _fps);
	if(_cam == NULL)		return;

	// Get camera frame dimensions
	CLEyeCameraGetFrameDimensions(_cam, w, h);

	// Depending on color mode chosen, create the appropriate OpenCV image
	if(_mode == CLEYE_COLOR_PROCESSED || _mode == CLEYE_COLOR_RAW)
		pCapImage = cvCreateImage(cvSize(w, h), IPL_DEPTH_8U, 4);
	else
		pCapImage = cvCreateImage(cvSize(w, h), IPL_DEPTH_8U, 1);

	// Set some camera parameters
	CLEyeSetCameraParameter(_cam, CLEYE_GAIN, gain);
	CLEyeSetCameraParameter(_cam, CLEYE_EXPOSURE, exposure);

	// Start capturing
	CLEyeCameraStart(_cam);
	cvGetImageRawData(pCapImage, &pCapBuffer);

	//////////////////////////////////////
	setupParams();
	//////////////////////////////////////

	// image capturing loop
	while(_running)
	{
		CLEyeCameraGetFrame(_cam, pCapBuffer);

		////////////
		showFPS();
		////////////

		//////////////////////////////////////
		colorFiltering();
		opening();
		find_ball();
		//////////////////////////////////////
	}

	// Stop camera capture
	CLEyeCameraStop(_cam);

	// Destroy camera object
	CLEyeDestroyCamera(_cam);

	// Destroy the allocated OpenCV image
	cvReleaseImage(&pCapImage);

	//////////////////////////////////
	destroyMyImages();
	//////////////////////////////////

	_cam = NULL;
}

// Main program entry point
int main(int argc, char* argv[])
{
	CLEyeCameraCapture *cam = NULL;
	if(CLEyeGetCameraCount() == 0){printf("No PS3Eye cameras detected\n");	return -1;}

	// Create camera capture object
	cam = new CLEyeCameraCapture("Camera Window", CLEyeGetCameraUUID(0), CLEYE_COLOR_RAW, CLEYE_VGA, 60);
	//cam = new CLEyeCameraCapture("Camera Window", CLEyeGetCameraUUID(0), CLEYE_COLOR_RAW, CLEYE_QVGA, 125);
	
	cam->StartCapture();

	// The <ESC> key will exit the program
	int key;
	while((key = cvWaitKey(0)) != 0x1b){}

	printf("Stopping capture on camera\n");
	cam->StopCapture();
	delete cam;

	return 0;
}

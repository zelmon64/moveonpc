#include "../include/MovePrecompiled.h"
#include "../include/EyeInterface.h"

namespace Eye
{

	EyeInterface::EyeInterface(void)
	{
		img=0;

		pCapBuffer=0;
		_cam=0;
		ballRegistered=false;
	}
	
	EyeInterface::~EyeInterface(void)
	{
		if (img)
			delete(img);

		if (_cam)
		{
			CLEyeCameraLED(_cam, false);
			CLEyeCameraStop(_cam);
			CLEyeDestroyCamera(_cam);
		}
		if (pCapBuffer)
			delete[] pCapBuffer;
	}

	void EyeInterface::RegisterBall()
	{
		img->RegisterBall();
		ballRegistered=true;
	}

	void EyeInterface::startCapture()
	{
		_hThread = CreateThread(NULL, 0, &EyeInterface::CaptureThread, this, 0, 0);
	}

	void EyeInterface::Run()
	{
		GUID _cameraGUID;

		_cameraGUID = CLEyeGetCameraUUID(0);
		_cam = CLEyeCreateCamera(_cameraGUID, CLEYE_COLOR_PROCESSED, CLEYE_VGA, 75);
		CLEyeCameraGetFrameDimensions(_cam, width, height);

		CLEyeSetCameraParameter(_cam, CLEYE_GAIN, 0);
		CLEyeSetCameraParameter(_cam, CLEYE_EXPOSURE, 160);
		CLEyeSetCameraParameter(_cam, CLEYE_AUTO_WHITEBALANCE, false);
		CLEyeSetCameraParameter(_cam, CLEYE_WHITEBALANCE_RED, 255);
		CLEyeSetCameraParameter(_cam, CLEYE_WHITEBALANCE_GREEN, 255);
		CLEyeSetCameraParameter(_cam, CLEYE_WHITEBALANCE_BLUE, 255);


		CLEyeCameraStart(_cam);
		Sleep(100);
		CLEyeCameraLED(_cam, true);

		pCapBuffer=new BYTE[width*height*4];
		img = new EyeImage(width,height,pCapBuffer);

		while(true)
		{
			CLEyeCameraGetFrame(_cam, pCapBuffer);

			float x,y,size;
			if (ballRegistered && img->findBall(x,y,size))
			{
				img->drawCircle((int)x,(int)y,(int)(size/2),ColorRgb(255,0,0));

				ballFound=true;
				ballZ=0.6*ballZ+0.4*(1000/size);
				ballX=0.2*ballX+0.8*(((float)x-(width/2))*(tan(1.308996939)*ballZ)/-140);
				ballY=0.2*ballY+0.8*(((float)y-(height/2))*(tan(1.308996939)*ballZ)/-140);
				ballZ-=5.0f;
			}
			else
			{
				ballFound=false;
			}
		}
	}

	DWORD WINAPI EyeInterface::CaptureThread(LPVOID instance)
	{
		EyeInterface *pThis = (EyeInterface *)instance;
		pThis->Run();
		return 0;
	}
}

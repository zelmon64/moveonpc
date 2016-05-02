#pragma once
#include "MovePrecompiled.h"

#include "EyeImage.h"

namespace Eye
{

	class EyeInterface
	{
		HANDLE _hThread;
		PBYTE pCapBuffer;
		CLEyeCameraInstance _cam;

		bool ballRegistered;
		int width, height;

	public:
		EyeImage *img;

	public:
		volatile float ballX,ballY,ballZ;
		volatile bool ballFound;

	public:
		EyeInterface(void);
		~EyeInterface(void);

		void RegisterBall();
		void startCapture();
		void getDimensions(int &x, int &y)
		{
			x=width; y=height;
		}

	private:
		void Run();
		static DWORD WINAPI CaptureThread(LPVOID instance);
		
	};
}

#pragma once

#include "MovePrecompiled.h"

namespace Eye
{
	struct ColorRgb
	{
		int r,g,b;
		ColorRgb()
			:r(0),g(0),b(0) {}
		ColorRgb(int r, int g, int b)
			:r(r),g(g),b(b) {}
	};


	struct ColorHsv
	{
		float h,s,v;
		ColorHsv()
			:h(0),s(0),v(0) {}
		ColorHsv(float h, float s, float v)
			:h(h),s(s),v(v) {}

		ColorHsv(ColorRgb rgb)
		{
			double r=((float)rgb.r)/255;
			double g=((float)rgb.g)/255;
			double b=((float)rgb.b)/255;

			// RGB are from 0..1, H is from 0..360, SV from 0..1
			double maxC = b;
			if (maxC < g) maxC = g;
			if (maxC < r) maxC = r;
			double minC = b;
			if (minC > g) minC = g;
			if (minC > r) minC = r;

			double delta = maxC - minC;

			v = maxC;
			s = 0;
			h = 0;

			if (delta == 0)
			{
				h = 0;
				s = 0;
			}
			else
			{
				s = delta / maxC;
				double dR = 60*(maxC - r)/delta + 180;
				double dG = 60*(maxC - g)/delta + 180;
				double dB = 60*(maxC - b)/delta + 180;
				if (r == maxC)
					h = dB - dG;
				else if (g == maxC)
					h = 120 + dR - dB;
				else
					h = 240 + dG - dR;
			}

			if (h<0)
				h+=360;
			if (h>=360)
				h-=360;
		}
	};

	struct EyeImage
	{
		PBYTE data;
		int w,h;
		ColorHsv *ballColor;
		int lastX,lastY;

		EyeImage(int w, int h, PBYTE imgBuffer);
		~EyeImage(void);

		void setPixel(int x, int y, ColorRgb rgb);
		ColorRgb getPixel(int x, int y);
		void drawCircle(int rx, int ry, int r, ColorRgb col);

		bool findBall(float &x, float &y, float &size);
		void RegisterBall();
		PBYTE getData();

	private:
		bool combImage(int &foundX, int &foundY);
		bool searchRow(int y, int &foundX, int &foundY);
		bool checkBallAtPosition(int x, int y);
		bool colorMatches(ColorRgb *pixelColor);
	};
}



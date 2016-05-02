#include "MovePrecompiled.h"
#include "EyeImage.h"

namespace Eye
{
	EyeImage::EyeImage(int w, int h, PBYTE data)
		:w(w),h(h),data(data)
	{
		ballColor=0;
		lastX=-1;
		lastY=-1;
	}


	EyeImage::~EyeImage(void)
	{
		if (ballColor)
			delete ballColor;
	}

	void EyeImage::setPixel(int i, int j, ColorRgb col)
	{
		if(i>=w || j>=h || i<0 || j<0)
			throw 0;
		data[j*w*4+i*4+0]=col.b; // B
		data[j*w*4+i*4+1]=col.g; // G
		data[j*w*4+i*4+2]=col.r; // R
	}

	ColorRgb EyeImage::getPixel(int i, int j)
	{
		ColorRgb col;
		if(i>=w || j>=h || i<0 || j<0)
		{
			throw 0;
		}
		col.b=data[j*w*4+i*4+0]; // B
		col.g=data[j*w*4+i*4+1]; // G
		col.r=data[j*w*4+i*4+2]; // R
		return col;
	}

	void EyeImage::drawCircle(int rx, int ry, int r, ColorRgb col)
	{
		int x,y;
		double d;//detail
		const double pi = 3.14159265; //more or less
		for(d=0; d<=2*pi; d+=0.01) //you can play with the value to be added to d
		{
			x=rx+sin(d)*r;
			y=ry+sin(d+(pi/2))*r;
			try
			{
				setPixel(x,y,col);
			}
			catch(int)
			{}
		}
	}

	void EyeImage::RegisterBall()
	{
		if (ballColor)
			delete ballColor;
		ColorRgb middle=getPixel(w/2,h/2);
		ballColor=new ColorHsv(middle);
	}

	PBYTE EyeImage::getData()
	{
		return data;
	}

	bool EyeImage::findBall(float &xx, float &yy, float &size)
	{
		if (!ballColor)
			return false;
		int x=0,y=0;
		ColorRgb pixel;
		if (lastX>0 && (pixel=getPixel(lastX,lastY), colorMatches(&pixel)))
		{
			x=lastX;
			y=lastY;
		}
		else if (!combImage(x,y))
		{
			lastX=-1;
			return false;
		}
		int searchX, searchY;
		int maxX,minX,maxY,minY;

		//searching for maxX
		searchX=x, searchY=y;
		pixel=getPixel(searchX,searchY);
		while(colorMatches(&pixel))
		{
			++searchX;
			try
			{
				pixel=getPixel(searchX,searchY);
			}
			catch (int)
			{
				break;
			}
		}
		maxX=searchX;

		//minX initially x
		minX=x;
		x=(minX+maxX)/2;


		//searching for maxY
		searchX=x, searchY=y;
		pixel=getPixel(searchX,searchY);
		while(colorMatches(&pixel))
		{
			++searchY;
			try
			{
				pixel=getPixel(searchX,searchY);
			}
			catch (int)
			{
				break;
			}
		}
		maxY=searchY;

		//searching for minY
		searchX=x, searchY=y;
		pixel=getPixel(searchX,searchY);
		while(colorMatches(&pixel))
		{
			--searchY;
			try
			{
				pixel=getPixel(searchX,searchY);
			}
			catch (int)
			{
				break;
			}
		}
		minY=searchY;
		y=(minY+maxY)/2;
		
		//searching for maxX
		searchX=x, searchY=y;
		pixel=getPixel(searchX,searchY);
		while(colorMatches(&pixel))
		{
			++searchX;
			try
			{
				pixel=getPixel(searchX,searchY);
			}
			catch (int)
			{
				break;
			}
		}
		maxX=searchX;

		//searching for minX
		searchX=x, searchY=y;
		pixel=getPixel(searchX,searchY);
		while(colorMatches(&pixel))
		{
			--searchX;
			try
			{
				pixel=getPixel(searchX,searchY);
			}
			catch (int)
			{
				break;
			}
		}
		minX=searchX;
		x=(minX+maxX)/2;


		size=(((float)(maxX-minX))+((float)(maxY-minY)))/2.0f;
		xx=(float)x;
		yy=(float)y;

		ColorRgb middle=getPixel(x,y);
		if (size>15 && colorMatches(&middle))
		{
			//update ball color
			if (ballColor)
				delete ballColor;
			ballColor=new ColorHsv(middle);
		}

		return true;
	}

	bool EyeImage::combImage(int &foundX, int &foundY)
	{
		float searchGap=(float)h/2.0f;
		if (searchRow((int)searchGap,foundX,foundY))
			return true;

		float searchY;
		while (searchGap>4)
		{
			searchY=searchGap/2;
			while ((int)searchY<h)
			{
				if (searchRow((int)searchY,foundX,foundY))
					return true;
				searchY+=searchGap;
			}
			searchGap/=2;
		}
		return false;
	}

	bool EyeImage::searchRow(int y, int &foundX, int &foundY)
	{
		//search in row
		for (int x=0;x<w;++x)
		{
			ColorRgb pCol=getPixel(x,y);
			if (colorMatches(&pCol))
			{
				if (checkBallAtPosition(x,y))
				{
					foundX=x; foundY=y;
					return true;
				}
			}
		}
		return false;
	}

	bool EyeImage::checkBallAtPosition(int x, int y)
	{
		return true;
	}

	bool EyeImage::colorMatches(ColorRgb *pixelColor)
	{
		ColorHsv pixelHsv=ColorHsv(*pixelColor);

		/*if (fabs(pixelHsv.h-ballColor->h)<30 &&
			fabs(pixelHsv.s-ballColor->s)<0.4 &&
			fabs(pixelHsv.v-ballColor->v)<0.4)
			return true;*/
		float x = ballColor->h;
		if (pixelHsv.v>0.6)
			return true;

		return false;
	}
}
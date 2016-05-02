/**
 * \brief  Detection of a colored sphere in an image (header file)
 * \author Alexander Nitsch <nitscha@cs.tu-berlin.de>
 *
 * This class provides tools for finding the enclosing circle of a colored
 * sphere in an RGB image.
 */

#pragma once

#include <stdio.h>
#include <stack>

#include <cv.h>
#include <highgui.h>

// TNT/JAMA for computation of eigenvalues
#include <jama_eig.h>


typedef struct {
	double x; // center's x-coordinate
	double y; // center's y-coordinate
	double r; // radius
} CircleParams;


class CircleTracker {
private:
	bool m_bShowMask;
	bool m_bShowContour;

public:
	CircleTracker()
		: m_bShowMask(false),
		  m_bShowContour(false)
	{ }
	
	~CircleTracker() {}

	/**
	 * minHue <= hue < maxHue
	 * In OpenCV values in the HSV hue channel are [0, 180) for 8-bit images.
	 */
	bool trackCircle(CircleParams *psCircle, IplImage *pImg,
		unsigned int uiMinHue, unsigned int uiMaxHue,
		bool bRemoveNoise = false, unsigned int uiFillDiff = 0);

	void setShowMask(bool b)    { m_bShowMask = b;    }
	void setShowContour(bool b) { m_bShowContour = b; }

private:
	void trackColor(IplImage *pImgMask, IplImage *pImg,
		unsigned int uiMinHue, unsigned int uiMaxHue,
		bool bRemoveNoise = false, unsigned int uiFillDiff = 0);

	void floodFill(IplImage *pImgMask, IplImage *pImg, unsigned int uiMaxDiff);

	CvSeq *chooseContour(IplImage *pImgMask, CvSeq *pContours, int iNumContours);

	bool fitCircle(CvSeq *pPointSeq, CircleParams *pCircle);
};

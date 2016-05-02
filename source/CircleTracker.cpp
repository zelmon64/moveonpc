/**
 * \brief  Detection of a colored sphere in an image
 * \author Alexander Nitsch <nitscha@cs.tu-berlin.de>
 */

#include "CircleTracker.h"


/**
 * Set pixels in BGR(A)-destination image to red if the corresponding pixels
 * in the first (and possibly only) channel of source are non-zero. This way,
 * source acts as a mask that controls which pixels to set to red in the
 * destination image.
 */
void addOverlay(IplImage *pImgDst, IplImage *pImgSrc) {
	for (int y = 0; y < pImgDst->height; y++) {
		unsigned char *pucData =
			(unsigned char *)(pImgSrc->imageData + y * pImgSrc->widthStep);
		for (int x = 0; x < pImgSrc->width; x++) {
			if (pucData[x]) {
				int xDst = pImgDst->nChannels * x;
				CV_IMAGE_ELEM(pImgDst, unsigned char, y, xDst + 0) =   0;
				CV_IMAGE_ELEM(pImgDst, unsigned char, y, xDst + 1) =   0;
				CV_IMAGE_ELEM(pImgDst, unsigned char, y, xDst + 2) = 255;
			}
		}
	}
}


bool CircleTracker::trackCircle(CircleParams *psCircle, IplImage *pImg,
		unsigned int uiMinHue, unsigned int uiMaxHue,
		bool bRemoveNoise, unsigned int uiFillDiff)
{
	bool bCircleFound = false;
	CvSize sImgSize = cvGetSize(pImg);
	IplImage *pImgMask     = cvCreateImage(sImgSize, IPL_DEPTH_8U, 1);
	IplImage *pImgMaskTemp = cvCreateImage(sImgSize, IPL_DEPTH_8U, 1);

	CvSeq *pContours = NULL;
	CvMemStorage *pMemStorage = cvCreateMemStorage();
	int iNumContours;

	// track circle by color (candidate points will be saved in pImgMask)
	trackColor(pImgMask, pImg, uiMinHue, uiMaxHue, bRemoveNoise, uiFillDiff);

	if (m_bShowMask) {
		// draw mask onto input image
		addOverlay(pImg, pImgMask);
	}

	// TODO: It might be much easier (and possibly faster) to use cvBlobsLib
	//       for the following process of finding blobs in the mask and
	//       filtering on the basis of their size etc.

	// find blob boundaries in mask (can be more than one blob!)
	// since this function modifies the image, we pass a copy
	cvCopy(pImgMask, pImgMaskTemp);
	iNumContours = cvFindContours(pImgMaskTemp, pMemStorage, &pContours,
		sizeof(CvContour), CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);

	if (iNumContours != 0) {
		CvSeq *pBlobContour = chooseContour(pImgMask, pContours, iNumContours);

		if (pBlobContour) {
			if (m_bShowContour) {
				// draw chosen blob's contour onto input image
				cvDrawContours(pImg, pBlobContour, CV_RGB(255,255,255), CV_RGB(0,0,0), 0);
			}

			// fit circle to boundary points
			if (fitCircle(pBlobContour, psCircle)) {
				// check if fitting returned valid coordinates and radius
				if ((psCircle->r > 0) && (psCircle->x >= 0) && (psCircle->y >= 0)) {
					bCircleFound = true;
				}
			}
		}
	}

	cvReleaseImage(&pImgMask);
	cvReleaseImage(&pImgMaskTemp);
	cvReleaseMemStorage(&pMemStorage);

	return bCircleFound;
}


void CircleTracker::trackColor(IplImage *pImgMask, IplImage *pImg,
		unsigned int uiMinHue, unsigned int uiMaxHue,
		bool bRemoveNoise, unsigned int uiFillDiff)
{
	IplImage *pImgHsv;
	IplImage *pImgV;

	// create HSV representation from BGR input image
	pImgHsv = cvCreateImage(cvGetSize(pImg), IPL_DEPTH_8U, 3);
	cvCvtColor(pImg, pImgHsv, CV_BGR2HSV);

	// build binary mask from HSV space:
	//
	// - The HUE determines which shade of color to match.
	//
	// - If SATURATION is close to zero the color will always be gray, no
	//   matter the HUE value. Therefore, a minimum SATURATION contraint is
	//   imposed to exclude gray pixels whose (arbitrary) HUE would otherwise
	//   result in a match.
	//
	// - Likewise, a VALUE close to zero will always result in black.
	//   Therefore, a minimum VALUE contraint is imposed to exclude near-black
	//   pixels whose (arbitrary) HUE would otherwise result in a match.
	//
	cvInRangeS(pImgHsv, cvScalar(uiMinHue, 50, 50), cvScalar(uiMaxHue, 256, 256), pImgMask);

	// keep only V channel for further processing
	pImgV = cvCreateImage(cvGetSize(pImg), IPL_DEPTH_8U, 1);
	cvSplit(pImgHsv, NULL, NULL, pImgV, NULL);
	cvReleaseImage(&pImgHsv);

	// TODO: Maybe we can do the following operations on the V channel using
	//       COI instead of copying the channel first. Need to check if the
	//       necessary functions support it!

	if (bRemoveNoise) {
		// remove noise from mask

		// A disc kernel often preserves round shapes better, but removes less
		// noise. So we'll go with the default 3x3 square kernel.
		int iIterations = 2;
		cvErode(pImgMask, pImgMask, NULL, iIterations);
		cvDilate(pImgMask, pImgMask, NULL, iIterations);
	}

	if (uiFillDiff > 0) {
		// perform flood fill in HSV value channel on mask to get neighbouring
		// pixels that might have been eliminated or missed in the previous
		// steps
		floodFill(pImgMask, pImgV, uiFillDiff);
	}

	cvReleaseImage(&pImgV);
}


void CircleTracker::floodFill(IplImage *pImgMask, IplImage *pImg,
		unsigned int uiMaxDiff)
{
	// - mask must have same size as image
	// - mask must have same number of channels as image

	std::stack<CvPoint> cSeeds;
	bool *pbProcessed;
	CvSize sImgSize = cvGetSize(pImg);
	unsigned int uiNumPixels;
	unsigned int uiSum;
	unsigned int uiRef;
	int iThreshLo;
	int iThreshUp;

	uiNumPixels = sImgSize.width * sImgSize.height;

	// find non-zero entries in mask and store their positions in list of seeds,
	// also calculate sum of corresponding pixel values in the image as we go
	uiSum = 0;
	for (int y = 0; y < pImgMask->height; y++) {
		unsigned char *pucData =
			(unsigned char *)(pImgMask->imageData + y * pImgMask->widthStep);
		for (int x = 0; x < pImgMask->width; x++) {
			if (pucData[x]) {
				cSeeds.push(cvPoint(x, y));
				uiSum += CV_IMAGE_ELEM(pImg, unsigned char, y, x);
			}
		}
	}

	// compute upper and lower thresholds for comparing neighbours
	uiRef = (unsigned int)(0.5f + (float)uiSum / cSeeds.size()); // rounded mean
	iThreshLo = uiRef - uiMaxDiff;
	iThreshUp = uiRef + uiMaxDiff;

	// init every pixel as not yet processed
	pbProcessed = new bool[uiNumPixels];
	memset(pbProcessed, 0, sizeof(bool) * uiNumPixels);
	
	while (!cSeeds.empty()) {
		// get and remove next pixel position from list of seeds
		CvPoint sPoint = cSeeds.top();
		cSeeds.pop();

		int p = sPoint.y * sImgSize.width + sPoint.x;

		if (!pbProcessed[p]) {
			// mark pixel as processed
			pbProcessed[p] = true;

			// check left neighbour
			if ((sPoint.x > 0) && !pbProcessed[p - 1]) {
				unsigned char ucPixel =
					CV_IMAGE_ELEM(pImg, unsigned char, sPoint.y, sPoint.x-1);
				if ((iThreshLo <= ucPixel) && (ucPixel <= iThreshUp)) {
					// add this neighbor to mask and to list of seeds
					cSeeds.push(cvPoint(sPoint.x-1, sPoint.y));
					CV_IMAGE_ELEM(pImgMask,
						unsigned char, sPoint.y, sPoint.x-1) = 255;
				}
			}

			// check right neighbour
			if ((sPoint.x < sImgSize.width - 1) && !pbProcessed[p + 1]) {
				unsigned char ucPixel =
					CV_IMAGE_ELEM(pImg, unsigned char, sPoint.y, sPoint.x+1);
				if ((iThreshLo <= ucPixel) && (ucPixel <= iThreshUp)) {
					// add this neighbor to mask and to list of seeds
					cSeeds.push(cvPoint(sPoint.x+1, sPoint.y));
					CV_IMAGE_ELEM(pImgMask,
						unsigned char, sPoint.y, sPoint.x+1) = 255;
				}
			}

			// check top neighbour
			if ((sPoint.y > 0) && !pbProcessed[p - sImgSize.width]) {
				unsigned char ucPixel =
					CV_IMAGE_ELEM(pImg, unsigned char, sPoint.y-1, sPoint.x);
				if ((iThreshLo <= ucPixel) && (ucPixel <= iThreshUp)) {
					// add this neighbor to mask and to list of seeds
					cSeeds.push(cvPoint(sPoint.x, sPoint.y-1));
					CV_IMAGE_ELEM(pImgMask,
						unsigned char, sPoint.y-1, sPoint.x) = 255;
				}
			}

			// check bottom neighbour
			if ((sPoint.y < sImgSize.height - 1) && !pbProcessed[p + sImgSize.width]) {
				unsigned char ucPixel =
					CV_IMAGE_ELEM(pImg, unsigned char, sPoint.y+1, sPoint.x);
				if ((iThreshLo <= ucPixel) && (ucPixel <= iThreshUp)) {
					// add this neighbor to mask and to list of seeds
					cSeeds.push(cvPoint(sPoint.x, sPoint.y+1));
					CV_IMAGE_ELEM(pImgMask,
						unsigned char, sPoint.y+1, sPoint.x) = 255;
				}
			}
		}
	}

	delete[] pbProcessed;
}


CvSeq *CircleTracker::chooseContour(IplImage *pImgMask, CvSeq *pContours,
		int iNumContours)
{
	CvSeq *pBlobContour = pContours;

	// TODO: Even if there is only one contour we need to check if this one
	//       makes sense i.e. if it is a sane candidate for the circle.

	// TODO: Handle cases where the blob is very close to the image borders,
	//       since in these situations the circle fitting will almost certainly
	//       report crap.

	// if there is more than one blob, choose the largest one (the one with the
	// greatest number of pixels enclosed by its contour)
	//
	// TODO: Use blobs' roundness as additional indicator to make
	//       this choice more robust.
	//
	// TODO: Add circle's location and size from last frame as optional
	//       parameter. This can make choosing the correct blob more
	//       reliable.
	if (iNumContours > 1) {
		IplImage *pImgBlob = cvCreateImage(cvGetSize(pImgMask), IPL_DEPTH_8U, 1);
		int iMaxNumPixels = 0;

		for (CvSeq *pCont = pContours; pCont != NULL; pCont = pCont->h_next) {
			CvRect blobBox = cvBoundingRect(pCont);

			cvSetImageROI(pImgBlob, blobBox);
			cvSetImageROI(pImgMask, blobBox);

			// find blob, i.e. all pixels enclosed by current contour
			cvZero(pImgBlob);
			cvDrawContours(pImgBlob, pCont, CV_RGB(255,255,255),
				CV_RGB(0,0,0), 0, CV_FILLED, 8,
				cvPoint(-blobBox.x, -blobBox.y));
			cvAnd(pImgBlob, pImgMask, pImgBlob);

			/*
			printf("points: %2d, area: %5.2f (%3d px), perimeter: %5.2f, " \
					"x: %2d, y: %2d, w: %2d, h: %2d\n",
				pCont->total, cvContourArea(pCont),
				cvCountNonZero(pImgBlob), cvContourPerimeter(pCont),
				blobBox.x, blobBox.y, blobBox.width, blobBox.height);

			// cvContourArea can be negative!
			*/

			int iNumPixels = cvCountNonZero(pImgBlob);
			if (iNumPixels > iMaxNumPixels) {
				// mark current blob as best choice
				iMaxNumPixels = iNumPixels;
				pBlobContour = pCont;
			}

			cvResetImageROI(pImgMask);
			cvResetImageROI(pImgBlob);
		}

		cvReleaseImage(&pImgBlob);
	}

	return pBlobContour;
}


/**
 * This implements least-square circle fitting based on the algebraic distance.
 * Read it up on http://homepages.inf.ed.ac.uk/rbf/CVonline/LOCAL_COPIES/FISHER/ALGDIST/alg.htm.
 *
 * TODO: This can (and needs to) be improved a lot. For instance, partial
 *       occlusion leads to dents in the contour that trip this algorithm up.
 *       This needs to be fixed to make the fitting more robust.
 */
bool CircleTracker::fitCircle(CvSeq *pPointSeq, CircleParams *pCircle) {
	// Sequences containing only a single pixel should not be passed to this
	// routine in the first place. Check for this case anyway since it results
	// in divisions by zero when constructing the scatter matrix.
	if (pPointSeq->total == 1) {
		return false;
	}

	CvPoint *p = (CvPoint *)cvGetSeqElem(pPointSeq, 0);
	int iMinX = p->x, iMinY = p->y;
	int iMaxX = 0,    iMaxY = 0;
	int iSumX = 0,    iSumY = 0;

	// get pixel coordinates, also calculate max, min and sum as we go
	//
	// TODO: Using the sequence macros for iteration might be faster.
	for (int i = 0; i < pPointSeq->total; i++) {
		p = (CvPoint *)cvGetSeqElem(pPointSeq, i);

		iSumX += p->x;
		iSumY += p->y;
		if (p->x < iMinX) { iMinX = p->x; }
		if (p->y < iMinY) { iMinY = p->y; }
		if (p->x > iMaxX) { iMaxX = p->x; }
		if (p->y > iMaxY) { iMaxY = p->y; }
	}

	// mean value of x and y coordinates respectively
	double mx = ((double)iSumX) / pPointSeq->total;
	double my = ((double)iSumY) / pPointSeq->total;

	TNT::Array2D<double> S(4, 4, 0.0);
	TNT::Array2D<double> V(4, 4);

	// build scatter matrix's upper right triangle and main diagonal
	//
	// TODO: Using the sequence macros for iteration might be faster.
	for (int i = 0; i < pPointSeq->total; i++) {
		p = (CvPoint *)cvGetSeqElem(pPointSeq, i);

		double x = p->x - mx;
		double y = p->y - my;
		double xx = x * x;
		double yy = y * y;
		double xxyy = xx + yy;

		S[0][0] += xxyy * xxyy; // (x^2 + y^2)^2
		S[0][1] += x * xxyy;    // x * (x^2 + y^2)
		S[0][2] += y * xxyy;    // y * (x^2 + y^2)
		S[0][3] += xxyy;        // x^2 + y^2
		S[1][1] += xx;          // x^2
		S[1][2] += x*y;         // x * y
		S[1][3] += x;           // x
		S[2][2] += yy;          // y^2
		S[2][3] += y;           // y
	}

	// normalise scatter matrix (since the matrix is symmetric the lower left
	// triangle can simply be copied from the upper right triangle)
	double sx = (iMaxX - iMinX) / 2.0;
	double sy = (iMaxY - iMinY) / 2.0;
	double s = (sx < sy) ? sy : sx;
	double sPow = s;
	S[1][3] = S[3][1] = S[1][3] / sPow;
	S[2][3] = S[3][2] = S[2][3] / sPow;
	sPow *= s; // s^2
	S[0][3] = S[3][0] = S[0][3] / sPow;
	S[1][1] = S[1][1] / sPow;
	S[1][2] = S[2][1] = S[1][2] / sPow;
	S[2][2] = S[2][2] / sPow;
	sPow *= s; // s^3
	S[0][1] = S[1][0] = S[0][1] / sPow;
	S[0][2] = S[2][0] = S[0][2] / sPow;
	sPow *= s; // s^4
	S[0][0] = S[0][0] / sPow;
	S[3][3] = pPointSeq->total;

	JAMA::Eigenvalue<double> cEigensolver(S);
	cEigensolver.getV(V);

	// calculate circle's center and radius from denormalised eigenvector
	// belonging to the smallest eigenvalue
	pCircle->x = -(V[1][0]*s - 2*V[0][0]*mx) / (2 * V[0][0]);
	pCircle->y = -(V[2][0]*s - 2*V[0][0]*my) / (2 * V[0][0]);
	pCircle->r = sqrt(
		(V[0][0]*(mx*mx + my*my) - V[1][0]*s*mx - V[2][0]*s*my - V[3][0]*s*s)
		/ V[0][0]
		- (pCircle->x * pCircle->x) - (pCircle->y * pCircle->y)
	);

	// TODO: Check for negative (or otherwise invalid) results and return
	//       false in these cases.

	return true;
}




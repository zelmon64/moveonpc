/**
 * \brief  Helper functions for vector and matrix manipulation (header file)
 * \author Alexander Nitsch <nitscha@cs.tu-berlin.de>
 *
 * OpenCV's functions are so incredibly slow for 3x3 matrixes that we use our
 * own routines for basic vector and matrix operations. They don't provide a
 * nifty interface, but they are fast (enough) and do the job.
 */

#pragma once

#include <math.h>


void normalise(double x[3]);

void cross(double x[3], double y[3], double out[3]);

void matrixMult(double A[3][3], double B[3][3], double out[3][3]);

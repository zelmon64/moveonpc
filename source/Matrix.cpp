/**
 * \brief  Helper functions for vector and matrix manipulation
 * \author Alexander Nitsch <nitscha@cs.tu-berlin.de>
 */

#include "Matrix.h"


void normalise(double x[3]) {
	double d = sqrt(x[0]*x[0] + x[1]*x[1] + x[2]*x[2]);
	x[0] /= d;
	x[1] /= d;
	x[2] /= d;
}


void cross(double x[3], double y[3], double out[3]) {
	double z[3];
	z[0] = x[1]*y[2] - x[2]*y[1];
	z[1] = x[2]*y[0] - x[0]*y[2];
	z[2] = x[0]*y[1] - x[1]*y[0];
	for (int i = 0; i < 3; i++) {
		out[i] = z[i];
	}
}


void matrixMult(double A[3][3], double B[3][3], double out[3][3]) {
	double temp[3][3];
	for (int i = 0; i < 3; i++) {
		for (int k = 0; k < 3; k++) {
			temp[i][k] = 0;
			for (int n = 0; n < 3; n++) {
				temp[i][k] += A[i][n] * B[n][k];
			}
		}
	}
	for (int i = 0; i < 3; i++) {
		for (int k = 0; k < 3; k++) {
			out[i][k] = temp[i][k];
		}
	}
}

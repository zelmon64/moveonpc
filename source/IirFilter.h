/**
 * \brief  Implementation of an infinite impulse response filter
 * \author Alexander Nitsch <nitscha@cs.tu-berlin.de>
 *
 * This class implements an infinite impulse response (IIR) filter described by
 * the equation
 *
 *   y(n) = b0*x(n) + b1*x(n-1) + ... + bN*x(n-N)
 *          - a1*y(n-1) - a2*y(n-2) - ... - aM*y(n-M)
 *
 * The current filter output y(n) is composed of a weighted sum of past output
 * samples y(n-1) ... y(n-M) and input samples x(n) ... x(n-N).
 */

#pragma once


class IirFilter {
private:
	double *m_pdFirCoeffs;
	double *m_pdIirCoeffs;
	double *m_pdInputBuf;
	double *m_pdOutputBuf;
	size_t m_uiNumFirCoeffs;
	size_t m_uiNumIirCoeffs;

public:
	IirFilter(double *pdFirCoeffs, size_t uiNumFirCoeffs,
			double *pdIirCoeffs, size_t uiNumIirCoeffs)
	{
		m_uiNumFirCoeffs = uiNumFirCoeffs;
		m_uiNumIirCoeffs = uiNumIirCoeffs;
		m_pdFirCoeffs = new double[uiNumFirCoeffs];
		m_pdInputBuf = new double[uiNumFirCoeffs];
		m_pdIirCoeffs = new double[uiNumIirCoeffs];
		m_pdOutputBuf = new double[uiNumIirCoeffs];

		memcpy(m_pdFirCoeffs, pdFirCoeffs, sizeof(double) * uiNumFirCoeffs);
		memset(m_pdInputBuf, 0, sizeof(double) * uiNumFirCoeffs);
		memcpy(m_pdIirCoeffs, pdIirCoeffs, sizeof(double) * uiNumIirCoeffs);
		memset(m_pdOutputBuf, 0, sizeof(double) * uiNumIirCoeffs);
	}

	~IirFilter() {
		delete[] m_pdFirCoeffs;
		delete[] m_pdInputBuf;
		delete[] m_pdIirCoeffs;
		delete[] m_pdOutputBuf;
	}

	double step(double input) {
		double sum = 0;

		for (size_t i = m_uiNumFirCoeffs - 1; i > 0; i--) {
			m_pdInputBuf[i] = m_pdInputBuf[i-1];
			sum += m_pdFirCoeffs[i] * m_pdInputBuf[i];
		}
		m_pdInputBuf[0] = input;
		sum += m_pdFirCoeffs[0] * m_pdInputBuf[0];

		for (size_t i = m_uiNumIirCoeffs - 1; i > 0; i--) {
			m_pdOutputBuf[i] = m_pdOutputBuf[i-1];
			sum -= m_pdIirCoeffs[i] * m_pdOutputBuf[i];
		}
		m_pdOutputBuf[0] = sum;

		return sum;
	}
};

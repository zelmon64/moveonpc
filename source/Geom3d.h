/**
 * \brief  Basic projective geometry tools (header file)
 * \author Alexander Nitsch <nitscha@cs.tu-berlin.de>
 */

#pragma once

#include <cv.h>
#include <vector>


typedef CvPoint3D64f       Vertex;
typedef std::pair<int,int> Edge;


class Object3d {
public:
	std::vector<Vertex> m_cVertices;
	std::vector<Edge>   m_cEdges;

public:
	Object3d() {};
	~Object3d() {};

	void addVertex(double x, double y, double z) {
		Vertex p = { x, y, z };
		m_cVertices.push_back(p);
	}

	void addEdge(int a, int b) {
		m_cEdges.push_back(Edge(a, b));
	}

	void translate(double x, double y, double z);
	void rotate(double R[3][3]);
};


class Geom3d {
private:
	double m_dFocalLength;
	int m_iOriginX;
	int m_iOriginY;

public:
	Geom3d(int xm, int ym, double focalLength)
		: m_iOriginX(xm),
		  m_iOriginY(ym),
		  m_dFocalLength(focalLength)
	{ }

	~Geom3d(void) {};

	CvPoint projectPoint(Vertex p);
	
	void plotObject(IplImage *pImg, Object3d *pcObject);
	void plotLine(IplImage *pImg, Vertex p0, Vertex p1,
			CvScalar color = CV_RGB(255, 255, 255), int thickness = 1);

	static Object3d *getBox(double w, double h, double d);
	static Object3d *getAxes(double w);
};

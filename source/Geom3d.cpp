/**
 * \brief  Basic projective geometry tools (header file)
 * \author Alexander Nitsch <nitscha@cs.tu-berlin.de>
 */

#include "Geom3d.h"

using namespace std;



void Object3d::translate(double x, double y, double z) {
	for (vector<Vertex>::iterator i = m_cVertices.begin(); i != m_cVertices.end(); i++) {
		i->x += x;
		i->y += y;
		i->z += z;
	}
}

void Object3d::rotate(double R[3][3]) {
	for (vector<Vertex>::iterator i = m_cVertices.begin(); i != m_cVertices.end(); i++) {
		Vertex p;
		p.x = R[0][0] * i->x + R[0][1] * i->y + R[0][2] * i->z;
		p.y = R[1][0] * i->x + R[1][1] * i->y + R[1][2] * i->z;
		p.z = R[2][0] * i->x + R[2][1] * i->y + R[2][2] * i->z;
		i->x = p.x;
		i->y = p.y;
		i->z = p.z;
	}
}



CvPoint Geom3d::projectPoint(Vertex p) {
	CvPoint i;

	// TODO: handle p.y == 0 to avoid division by zero!
	p.y += 1;
	
	// project physical point into the camera
	i.x = (int)(m_iOriginX + (m_dFocalLength * (p.x / p.y)));
	i.y = (int)(m_iOriginY - (m_dFocalLength * (p.z / p.y)));

	return i;
}


void Geom3d::plotLine(IplImage *pImg, Vertex p0, Vertex p1, CvScalar color, int thickness) {
	// project physical points into the camera
	CvPoint i = projectPoint(p0);
	CvPoint j = projectPoint(p1);

	// draw line in camera plane
	cvLine(pImg, i, j, color, thickness);
}


void Geom3d::plotObject(IplImage *pImg, Object3d *pcObj) {
	for (vector<Edge>::iterator i = pcObj->m_cEdges.begin(); i != pcObj->m_cEdges.end(); i++) {
		plotLine(pImg, pcObj->m_cVertices[i->first], pcObj->m_cVertices[i->second]);
	}
}


Object3d *Geom3d::getBox(double w, double h, double d) {
	Object3d *pcBox = new Object3d;

	pcBox->addVertex(-w,  0, -h);
	pcBox->addVertex( w,  0, -h);
	pcBox->addVertex(-w,  0,  h);
	pcBox->addVertex( w,  0,  h);
	pcBox->addVertex(-w,  d, -h);
	pcBox->addVertex( w,  d, -h);
	pcBox->addVertex(-w,  d,  h);
	pcBox->addVertex( w,  d,  h);

	pcBox->addEdge(0, 1);
	pcBox->addEdge(1, 3);
	pcBox->addEdge(3, 2);
	pcBox->addEdge(2, 0);
	pcBox->addEdge(4, 5);
	pcBox->addEdge(5, 7);
	pcBox->addEdge(7, 6);
	pcBox->addEdge(6, 4);
	pcBox->addEdge(0, 4);
	pcBox->addEdge(1, 5);
	pcBox->addEdge(2, 6);
	pcBox->addEdge(3, 7);

	return pcBox;
}


Object3d *Geom3d::getAxes(double w) {
	Object3d *pcAxes = new Object3d;

	pcAxes->addVertex(0, 0, 0);
	pcAxes->addVertex(w, 0, 0);
	pcAxes->addVertex(0, w, 0);
	pcAxes->addVertex(0, 0, w);

	pcAxes->addEdge(0, 1);
	pcAxes->addEdge(0, 2);
	pcAxes->addEdge(0, 3);

	return pcAxes;
}


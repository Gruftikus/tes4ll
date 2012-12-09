#ifndef _PLLQUADLIST_H_
#define _PLLQUADLIST_H_

#include "../externals/niflib/include/niflib.h"
#include "../include/lllogger.h"

using namespace Niflib;


class llQuad {

private:

	int npoints, maxpoints;
	std::vector<int> points;
	std::vector<float> points_x;
	std::vector<float> points_y;

public:

	float x1,y1,x2,y2; //corner coordinates
	int x, y; //current position in the 2d-grid

	//constructor
	llQuad(int _x, int _y, float _x1, float _y1, float _x2, float _y2);
	llQuad();

	void SetMaxPoints(int _maxpoints) {
		maxpoints = _maxpoints;
	};

	int FreePoints(void) {
		return maxpoints - npoints;
	};

	int GetNumPoints(void) {
		return npoints;
	};

	void AddPoint(float _x, float _y, int _point_num) {
		if (npoints == points.size()) {
			points.resize(npoints+1000);
			points_x.resize(npoints+1000);
			points_y.resize(npoints+1000);
		}
		points[npoints]   = _point_num;
		points_x[npoints] = _x;
		points_y[npoints] = _y;
		npoints++;
	};

	int GetPointNum(int _i) {
		return points[_i];
	}

	float GetPointX(int _i) {
		return (points_x[_i]);
	}
	float GetPointY(int _i) {
		return (points_y[_i]);
	}

	int GetPoint(float _x, float _y) {
		for (unsigned int i=0; i<points.size();i++) {
			if (fabs(points_x[i] - _x) < 1.f && fabs(points_y[i] - _y) < 1.f) return points[i];
		}
		return -1;
	}

	int GetMinDistance(float *_min, float _x, float _y, float _radius);

};

class llQuadList {

private:

	std::vector<llQuad> v;
	unsigned int counter;
	unsigned int pointer;
	llLogger *mesg;
	llQuadList *subtree;

public:

	//constructor
	llQuadList(llLogger * _mesg, int _n);
	llQuadList(llLogger * _mesg, int _pos_x, int _pos_y, int _x, int _y, float _x1, float _y1, float _x2, float _y2);

	void SetMaxPoints(int _maxpoints) {
		for (unsigned int i=0;i<counter;i++) {
			v[i].SetMaxPoints(_maxpoints);
		}
	};


	int AddPoint(float _x, float _y, int _num); 

	int AddQuad(int _p1, int _p2, float _x1, float _y1, float _x2, float _y2);

	void Reset(void) {
		pointer=0;
	};

	int GetNextQuad(void) {
		pointer++;
		if (pointer>=counter) return 0;
		return 1;
	};

	unsigned int GetNumQuads(void) {
		return v.size();
	};

	llQuad * GetQuad(float _x, float _y, int _num = -1);

	llQuad * GetQuad(unsigned int _n) {
		return &(v[_n]);
	}
	
	void SubQuadLevels(int _levels);

	int GetCurrentX(void) {
		return v[pointer].x;
	}

	int GetCurrentY(void) {
		return v[pointer].y;
	}
};

#endif

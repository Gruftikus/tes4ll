#ifndef _PLLPOLYGONLIST_H_
#define _PLLPOLYGONLIST_H_

#include "../include/llpointlist.h"
#include "../include/lllogger.h"
#include "../include/llmap.h"

class llPolygon {

private:

	std::vector<int> p; //points of polygon
	
	llPointList *points;

public:

	llPolygon(int _n1, int _n2, char *_name, llPointList *_r);
	char *name;
	int AddPoint(int _n);
	int GetPoint(unsigned int _n) {if (_n<p.size()) return p[_n];return -1;};
	int GetSize(void) {return p.size();};

	int IsPointInsidePolygon(float _x, float _y);

};

class llPolygonList {

private:

	std::vector<llPolygon*> p;
	llLogger *mesg;
	llPointList * points;
	llMap *map;

public:

	llPolygonList(llLogger *_mesg, llPointList *_r, llMap *_map);

	llPolygon * GetPolygon(char *_name);
	llPolygon * GetPolygon(unsigned int _n) {return p[_n];};
	int GetSize(){return p.size();};

	//int AddPolygon(int _n1, int _n2, char *_name);
	int AddPolygon(float _x1, float _y1, float _x2, float _y2, char *_name);

	int AddVertexToPolygon(float _x, float _y, char *_name);    	
};


#endif
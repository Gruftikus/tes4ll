#include "..\include\llpolygonlist.h"

llPolygon::llPolygon(int _n1, int _n2, char *_name, llPointList *_r) {
	p.resize(2);
	p[0] = _n1;
	p[1] = _n2;
	name = _name;
	points = _r;
}

int llPolygon::AddPoint(int _n) {

	p.resize(p.size()+1);
	p[p.size()-1] = _n;

	return 1;
}

int llPolygon::IsPointInsidePolygon(float _x, float _y) {
	//Raycasting algo: http://en.wikipedia.org/wiki/Point_in_polygon
	int num=0;
	int v = points->GetPoint(_x, _y);
	for (int i = 0; i < GetSize();i++) {
		int i1= GetPoint(i);
		int i2= GetPoint(0);//close the poly
		if (i<GetSize()-1)
			i2= GetPoint(i+1);

		if ((i2 == v || i1 == v) && v != -1) return 1;

		float x1 = points->GetX(i1);
		float y1 = points->GetY(i1);
		float x2 = points->GetX(i2);
		float y2 = points->GetY(i2);
				
		if ( (y1<=_y && y2>_y)  ||  (y1>=_y && y2<_y) ) {
			//crosses x-axis
			float x_cross = x1 + (x2-x1) * ((_y-y1)/(y2-y1));
			if ((num && x_cross <= _x) || (x_cross < _x)) num++;
		} 
	}
	if (!num) return 0;
	if (num%2) {
		return 1;
	}
	return 0;
}



llPolygonList::llPolygonList(llLogger *_mesg, llPointList *_points, llMap *_map) {
	points = _points;
	map    = _map;
	mesg   = _mesg;
	p.resize(0);
}

int llPolygonList::AddPolygon(float _x1, float _y1, float _x2, float _y2, char *_name) {

	if (GetPolygon(_name)) {
		mesg->WriteNextLine(LOG_ERROR,"Polygon (%s) already existing",_name);
		return 0;
	}

	char *myname2 = new char[strlen(_name) + 1];
	strcpy_s(myname2,strlen(_name)+1,_name);

	int point1 = points->GetPoint(_x1, _y1);
	if (point1<0)
		point1=points->AddPoint(_x1, _y1, map->GetZ(_x1, _y1));
	int point2 = points->GetPoint(_x2, _y2);
	if (point2<0)
		point2=points->AddPoint(_x2, _y2, map->GetZ(_x2, _y2));
	
	p.resize(p.size()+1);

	llPolygon * mypoly = new llPolygon(point1, point2, myname2, points);
	p[p.size()-1]=mypoly;

	return 1;
}

int llPolygonList::AddVertexToPolygon(float _x, float _y, char *_name) {

	llPolygon * mypoly = GetPolygon(_name);

	if (!mypoly) {
		mesg->WriteNextLine(LOG_ERROR,"Polygon (%s) not existing",_name);
		return 0;
	}

	int point1 = points->GetPoint(_x, _y);
	if (point1<0)
		point1=points->AddPoint(_x, _y, map->GetZ(_x, _y));

	mypoly->AddPoint(point1);

	return 1;
}

llPolygon * llPolygonList::GetPolygon(char *_name) {
	for (unsigned int i=0; i < p.size(); i++) {
		if (_stricmp(_name,p[i]->name)==0) {
			return p[i];
		}
	}
	return NULL;
}



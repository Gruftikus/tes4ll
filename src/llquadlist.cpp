#include "..\include\llquadlist.h"

llQuad::llQuad(int _x, int _y, float _x1, float _y1, float _x2, float _y2) {

    x  = _x;
    y  = _y;
	x1 = _x1;
	y1 = _y1;
	x2 = _x2;
	y2 = _y2;

	if (x1 > x2 || y1 > y2) {
		cout << "Order of coordinates not correct" << endl;
	} 

    npoints   = 0;
    maxpoints = 65535;
}

int llQuad::GetMinDistance(float *_min, float _x, float _y, float _radius) {
	int my_case = 1; //check quad (default)
	if (_x+_radius < x1 && _x-_radius > x2 && _y+_radius < y1 && _y-_radius > y2) {
		//no hope that the circle will ever touch this quad
		my_case = 0;
	} //TODO: subquads
	if (my_case) {
		for (unsigned int j=0; j < points.size(); j++) {
			float my_x = _x - points_x[j];
			float my_y = _y - points_y[j];
			my_x *= my_x;
			my_y *= my_y;
			float minnew = my_x + my_y;

			if (minnew < *_min) *_min = minnew;
		}
	}
	return 1;
}

llQuad::llQuad() {

}


//constructor
llQuadList::llQuadList(llLogger * _mesg, int _n) {
    v.resize(_n);
    counter = 0;
	pointer = 0;
	mesg    = _mesg;
	subtree = NULL;
} 

llQuadList::llQuadList(llLogger * _mesg, int _pos_x, int _pos_y, int _x, int _y, float _x1, float _y1, float _x2, float _y2) {
	v.resize(_x * _y);
	counter = 0;
	pointer = 0;
	mesg    = _mesg;
	subtree = NULL;

	for (int ix=_pos_x; ix < _pos_x+_x; ix++) {
		for (int iy=_pos_y; iy < _pos_y+_y; iy++) {
			AddQuad(ix,iy,float(ix)*(_x2-_x1)/float(_x),
				float(iy)*(_y2-_y1)/float(_y),
				float(ix+1)*(_x2-_x1)/float(_x),
				float(iy+1)*(_y2-_y1)/float(_y));
		}
	}
} 

int llQuadList::AddQuad(int _p1, int _p2, float _x1, float _y1, float _x2, float _y2) {
	if (counter>=v.size()) {
		v.resize(v.size() + 1);
	}

	v[counter] = llQuad(_p1, _p2, _x1, _y1, _x2, _y2);
	counter++;
	return counter-1;
}

llQuad * llQuadList::GetQuad(float _x, float _y, int _num) {

	int num = _num;
	if (_num<0) {
		//direct search possible?
		//if (v.size() == counter) {
			//all quads there.....
			//ewrewrewrewr
		//}
		num=0;
	}
	for (unsigned int i=0;i<counter;i++) {
		if (v[i].x1-0.5f <= _x && v[i].x2+0.5f >= _x && v[i].y1-0.5f <= _y && v[i].y2+0.5f >= _y) {
			if (!num)
				return &(v[i]);
			else 
				num--;
		}
	}
	if (_num<0) {
		mesg->WriteNextLine(LOG_FATAL,"Quad not found (x=%f ,y=%f)",_x,_y);
		mesg->Dump();
		exit(1);
	}
	return NULL;
};

void llQuadList::SubQuadLevels(int _levels) {
	if (!_levels) return;
	subtree = new llQuadList(mesg, v.size()*4);
	if (subtree) {
		subtree->SubQuadLevels(_levels - 1);
	} else {
		mesg->WriteNextLine(LOG_FATAL,"Allocation of subtree failed (out of memory?)");
		exit(-1);
	}
}



int llQuadList::AddPoint(float _x, float _y, int _num) {
	llQuad * myquad=GetQuad(_x, _y, 0);
	int num=0;
	while (myquad) {
		if (myquad->FreePoints()<0) return 0;
		else {
			myquad->AddPoint(_x, _y, _num); //for statistics and fast quadtree search
			num++;
			myquad=GetQuad(_x, _y, num);
		}
	} 
	if (subtree) return subtree->AddPoint(_x, _y, _num);
	return 1;
};
#include "..\include\llmap.h"

//constructor
llMap::llMap(unsigned long _x, unsigned long _y, int _makeshort, float _default) {
	widthx=_x;
	widthy=_y;
	mesg = _llLogger();

	makeshort = _makeshort;
    data = new llShortarray(widthx*widthy,makeshort, _default); 
	data1x = data1y = data2x = data2y = NULL;

    der_done=false;
	scaling=1;
	default = _default;
}

llMap * llMap::Clone(int _expand, int _makeshort) {
	llMap *dummy = new llMap(widthx*_expand, widthy*_expand, makeshort, default);
	dummy->SetCoordSystem(x1,y1,x2,y2);
	return dummy;
}

llMap::~llMap() {

    delete data; 
	
    if (data1x)  delete data1x;
    if (data1y)  delete data1y;
	if (data2x)  delete data2x;
    if (data2y)  delete data2y;
}

void llMap::SetPoint(unsigned int _x, unsigned int _y, int _val) {
    data->SetElement(_x + _y*widthx, float(_val));
}

llMap * llMap::Filter(unsigned long _dist, int _overwrite, llCommands *_batch) {

	llMap *tmp= new llMap(widthx, widthy, makeshort, default);
	float minheight = default + 1.0f;

	for (unsigned long y=0;y<widthy;y++) {
		for (unsigned long x=0;x<widthx;x++) {
			unsigned long  x1=x-_dist;
			unsigned long  x2=x+_dist;
			unsigned long  y1=y-_dist;
			unsigned long  y2=y+_dist;
			if (x1<0) x1=0;
			if (y1<0) y1=0;
			if (x2>(widthx-1)) x2=widthx-1;
			if (y2>(widthy-1)) y2=widthy-1;
			float mean=0.,num=0.;
			for (unsigned long  xx=x1;xx<=x2;xx++) {
				for (unsigned long  yy=y1;yy<=y2;yy++) {
					float height = (float)GetElement(xx,yy);
					if (height > minheight) {
						mean +=  height;
						num++;
					}
				}
			}
			if (num)
				tmp->SetElement(x,y,mean/num);
			else
				tmp->SetElement(x,y,default);
		}
	}
	tmp->SetCoordSystem(x1,y1,x2,y2);
	
	if (_overwrite) {
		for (unsigned long y=0;y<widthy;y++) {
			for (unsigned long x=0;x<widthx;x++) {
				this->SetElement(x,y,tmp->GetElement(x,y));
			}
		}
		delete tmp;
		tmp = this;
	}
	
	return tmp;
}


void llMap::MakeDerivative(int _use16bit) {

	if (der_done) return;
	der_done=true;

	float minheight = (default*8.0f) + 1.0f;
	int redone = 0;

repeat:

	if (!data1x) {
		data1x = new llShortarray(widthx*widthy, _use16bit); 
		data1y = new llShortarray(widthx*widthy, _use16bit); 	
		data2x = new llShortarray(widthx*widthy, _use16bit); 	
		data2y = new llShortarray(widthx*widthy, _use16bit); 
	}

	if (!data1x->SetElement(0,0.f) || !data1y->SetElement(0,0.f) || 
		!data2x->SetElement(0,0.f) || !data2y->SetElement(0,0.f)) { //test bad alloc
		if (!_use16bit) {
			_use16bit = 1;
			redone    = 1;
			mesg->WriteNextLine(LOG_WARNING,"llMap::MakeDerivative: memory allocation failed, I will try the short version");
			delete data1x;
			delete data1y;
			delete data2x;
			delete data2y;
			data1x = data1y = data2x = data2y = NULL;
			goto repeat;
		} else {
			mesg->WriteNextLine(-LOG_FATAL,"llMap::MakeDerivative: out of memory");
			exit(1);
		}
	}

	if (redone) {
		mesg->AddToLine("... [OK]");
		mesg->Dump();
	}


	x1max=0;
	for (unsigned long y=0;y<widthy;y++) {
		data1x->SetElement(y*widthx,(GetZCoord(1,y)-GetZCoord(0,y)));
		data1x->SetElement(y*widthx+widthx-1,(GetZCoord(widthx-1,y)-GetZCoord(widthx-2,y)));
		for (unsigned long  x=1;x<widthx-1;x++) {
			if ((GetZCoord(x-1,y) > minheight && GetZCoord(x+1,y) > minheight)) {
				data1x->SetElement(x+y*widthx,(GetZCoord(x-1,y) - GetZCoord(x+1,y)));
				if (fabs(((*data1x)[x+y*widthx]))>x1max) x1max=fabs(((*data1x)[x+y*widthx]));
			}
		}
	}

	y1max=0;
	for (unsigned long  x=0;x<widthx;x++) {
		data1y->SetElement(x,(GetZCoord(x,1)-GetZCoord(x,0)));
		data1y->SetElement(widthx+(widthy-1)*widthx,(GetZCoord(x,widthy-1)-GetZCoord(x,widthy-2)));
		for (unsigned long  y=1;y<widthy-1;y++) {
			if ((GetZCoord(x,y-1) > minheight && GetZCoord(x,y+1) > minheight)) {
				data1y->SetElement(x+y*widthx,(GetZCoord(x,y-1)-GetZCoord(x,y+1)));
				if (fabs(((*data1y)[x+y*widthx]))>y1max) y1max=fabs(((*data1y)[x+y*widthx]));
			}
		}
	}

	x2max=0;
	for (unsigned long  y=0;y<widthy;y++) {
		data2x->SetElement(y*widthx,fabs((GetX1Coord(1,y)-GetX1Coord(0,y))) + fabs((GetY1Coord(1,y)-GetY1Coord(0,y))));
		data2x->SetElement(y*widthx+widthx-1,fabs((GetX1Coord(widthx-1,y)-GetX1Coord(widthx-2,y))) + fabs((GetY1Coord(widthx-1,y)-GetY1Coord(widthx-2,y))));
		for (unsigned long  x=1;x<widthx-1;x++) {
			if ((GetZCoord(x-1,y) > minheight && GetZCoord(x+1,y) > minheight)) {
				data2x->SetElement(x+y*widthx,fabs((GetX1Coord(x-1,y)-GetX1Coord(x+1,y)))  +  fabs((GetY1Coord(x-1,y)-GetY1Coord(x+1,y))));
				if (fabs(((*data2x)[x+y*widthx]))>x2max) x2max=fabs(((*data2x)[x+y*widthx]));
			}
		}
	}

	y2max=0;
	for (unsigned long  x=0;x<widthx;x++) {
		data2y->SetElement(x,fabs((GetX1Coord(x,1)-GetX1Coord(x,0))) + fabs((GetY1Coord(x,1)-GetY1Coord(x,0))));
		data2y->SetElement(widthx+(widthy-1)*widthx,fabs((GetX1Coord(x,widthy-1)-GetX1Coord(x,widthy-2)))  +  fabs((GetY1Coord(x,widthy-1)-GetY1Coord(x,widthy-2))));
		for (unsigned long  y=1;y<widthy-1;y++) {
			if ((GetZCoord(x,y-1) > minheight && GetZCoord(x,y+1) > minheight)) {
				data2y->SetElement(x+y*widthx,fabs((GetX1Coord(x,y-1)-GetX1Coord(x,y+1))) +  fabs((GetY1Coord(x,y-1)-GetY1Coord(x,y+1))));
				if (fabs(((*data2y)[x+y*widthx]))>y2max) y2max=fabs(((*data2y)[x+y*widthx]));
			}
		}
	}

	data1x->Print("1st order derivative, x-direction", mesg);
	data1y->Print("1st order derivative, y-direction", mesg);
	data2x->Print("2nd order derivative, x-direction", mesg);
	data2y->Print("2nd order derivative, y-direction", mesg);

}

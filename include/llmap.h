#ifndef _PLLMAP_H_
#define _PLLMAP_H_


#include <math.h>
#include <iostream>

#include "../include/lllogger.h"
#include "../include/llcommands.h"


class llShortarray {

private:

	short *my_shortarray;
	float *my_longarray;
	int    mysize;
	int    out_of_bounds, num_total;

public:

	llShortarray();
	~llShortarray() {
		if (my_shortarray) delete[] my_shortarray;
		if (my_longarray)  delete[] my_longarray;
	}

	llShortarray(int _size, int _makeshort = 0, float _default = 0) {
		my_shortarray = NULL;
		my_longarray  = NULL;
		if (_makeshort)
			my_shortarray = new (std::nothrow) short[_size];
		else
			my_longarray  = new (std::nothrow) float[_size];

		if (my_shortarray || my_longarray) {
			mysize        = _size;
			out_of_bounds = num_total = 0;
			for (int i=0; i < _size ; i++) {
				SetElement(i, _default);
			}
		} else {
			mysize = 0;
		}

	}

	int SetElement(int _x, float _val) {

		if ((!my_shortarray && !my_longarray) || !mysize) return 0;

		if (my_longarray) {
			my_longarray[_x] = _val;
			return 1;
		}
		if (_val > 122872  && my_shortarray) {out_of_bounds++;_val=122872;}
		if (_val < -122880 && my_shortarray) {out_of_bounds++;_val=-122880;}	
		
		num_total++;

		float mult=1;
		if (_val<0) mult=-1;
		if (fabs(_val)<8192.5)
			my_shortarray[_x]=short(_val);
		else if (fabs(_val) < 24574.5)
			my_shortarray[_x]=short((_val - mult*8192)/2 + mult*8192);
		else if (fabs(_val) < 57340.5)
			my_shortarray[_x]=short((_val - mult*24576)/4 + mult*16384);
		else 
			my_shortarray[_x]=short((_val - mult*57344)/8 + mult*24576);
		return 1;
	}

	void Print(char *_st, llLogger *_mesg) {
		if (_mesg) {
			if (my_longarray)  _mesg->WriteNextLine(LOG_INFO,"Array '%s' has 32 bit floats",_st);
			if (my_shortarray) _mesg->WriteNextLine(LOG_INFO,"Array '%s' has 16 bit shorts",_st);
			if (out_of_bounds) {
				_mesg->WriteNextLine(LOG_INFO,"Array '%s' has %i elements, out of which %i had to be truncated",_st,num_total,out_of_bounds);
			}
		} else {
			if (my_longarray)  std::cout << "[Info] Array '"<< _st <<"' has 32 bit floats" <<  std::endl;
			if (my_shortarray) std::cout << "[Info] Array '"<< _st <<"' has 16 bit shorts" <<  std::endl;
			if (out_of_bounds) {
				std::cout << "[Info] Array '"<< _st <<"' has "  << num_total 
					<< " elements, out of which " << out_of_bounds  <<  " had to be truncated" <<  std::endl;
			}
		}
	}

	float operator[] (int _x) {
		if (_x > mysize) {
			std::cout << "[Fatal] Index "  << _x << " out of bounds" <<  std::endl;
			exit(1);
		}
		if (my_longarray) return my_longarray[_x];
		float val;

		// Range
// 0-8191      : 0-8191
// 8192-16383  : 8192-24574 (x2)
// 16384-24575 : 24576-57340 (x4)
// 24576-32767 : 57344-122880 (x8)
		
		short y = my_shortarray[_x];
		short mult = 1;
		if (y<0) mult=-1;

		if (y<=8192 && y>=-8192)
			val = float(y);
		else if (y<=16383 && y>=-16383)
			val = float((y-mult*8192)*2 + mult*8192);
		else if (y<=24575 && y>=-24575)
			val = float((y-mult*16384)*4 + mult*24576);
		else
			val = float((y-mult*24576)*8 + mult*57344);
		return val;
	}
	
};

class llMap {

protected:

	unsigned int  widthx,   widthy;
	float         f_widthx, f_widthy;
	float         widthx_per_raw, widthy_per_raw;

	float x1max;
	float y1max;
	float x2max;
	float y2max;

	llShortarray *data1x;
	llShortarray *data1y;
	llShortarray *data2x;
	llShortarray *data2y;

	float x1, y1, x2, y2, scaling;
	bool der_done;

	llShortarray *data;
	float default;
	int makeshort;
	llLogger *mesg;

public:

	llMap(unsigned long _x = 0, unsigned long _y = 0, int _makeshort=0, float _default = 0);
	~llMap();
	llMap * Clone(int _expand, int _makeshort);

	//Raw:
	unsigned int GetWidthX(void) {return widthx;};
	unsigned int GetWidthY(void) {return widthy;};

	void SetElementRaw(unsigned int _x, unsigned int _y, float _val) {
		//std::cout << _x << ":" << _y << ":" << _val << std::endl;
		if (_x >= widthx || _y >= widthy) return;
		data->SetElement(_x+_y*widthx, _val);
	};
	void SetElementRaw(unsigned int _x, unsigned int _y, int _val) {
		if (_x >= widthx || _y >= widthy) return;
		data->SetElement(_x+_y*widthx, float(_val));
	};
	float GetElementRaw(unsigned int _x, unsigned int _y) {
		if (_x >= widthx || _y >= widthy) return default;
		return (*data)[_x+_y*widthx];
	};
	void ChangeElementRaw(unsigned int _x, unsigned int _y, float _val) {
		if (_x >= widthx || _y >= widthy) return;
		data->SetElement(_x + _y*widthx, (*data)[_x+_y*widthx] + _val);
	};
	
	//With coordinates:
	void SetCoordSystem(float _x1, float _y1, float _x2, float _y2) {
		x1 = _x1;
		y1 = _y1;
		x2 = _x2;
		y2 = _y2;
		widthx_per_raw = (x2 - x1) / f_widthx;
		widthy_per_raw = (y2 - y1) / f_widthy;
	}
	float GetX1() {return x1;};
	float GetY1() {return y1;};
	float GetX2() {return x2;};
	float GetY2() {return y2;};

	//Transformation:
	unsigned int GetRawX(float _x) {
		if (_x < x1) return 0;
		unsigned int xx = unsigned int((_x - x1) * f_widthx/(x2-x1));
		if (xx) return xx - 1; //correct small offset
		return xx;
	}
	unsigned int GetRawY(float _y) {
		if (_y < y1) return 0;
		unsigned int yy = unsigned int((_y - y1) * f_widthy/(y2-y1));
		if (yy) return yy - 1; //correct small offset
		return yy;
	}
	float GetCoordX(unsigned int _x) {
		return ((float)_x*(x2-x1)/f_widthx + x1);
	}

	float GetCoordY(unsigned int _y) {
		return ((float)_y*(y2-y1)/f_widthy + y1);
	}
	float GetWidthXPerRaw() {
		return widthx_per_raw;
	};
	float GetWidthYPerRaw() {
		return widthy_per_raw;
	};

#if 0
	int GetX(int _x) {
		int xx = int((double(_x) - double(x1)) * double(widthx)/((double)x2-(double)x1))-1;
		if (xx<0) xx=0;
		return xx;
	}
	int GetY(int _y) {
		int yy = int((double(_y) - double(y1)) * double(widthy)/((double)y2-(double)y1))-1;
		if (yy<0) yy=0;
		return yy;
	}
#endif

	//float GetZCoord(unsigned int _x, unsigned int _y) {
	float GetZ(unsigned int _x, unsigned int _y) {
		if (_x<0 || _x>=widthx || _y<0 || _y>=widthy) return 8*default; 
		return 8.0f*(*data)[_x + _y*widthx];
	}

#if 0
	float GetZ(int _x, int _y) {
		unsigned int x1 = GetRawX(_x);
		unsigned int y1 = GetRawY(_y);
		if (x1<0 || x1>=widthx || y1<0 || y1>=widthy) return 8*default;
		return 8.0f*(*data)[_x + _y*widthx];
	}
#endif

	float GetZ(float _x, float _y) {
		unsigned int x1 = GetRawX(_x);
		unsigned int y1 = GetRawY(_y);
		if (x1>=widthx || y1>=widthy) return 8*default; 
		return (8.0f*(*data)[x1+y1*widthx]);
	}

	float GetZ(double _x, double _y) {
		unsigned int x1 = GetRawX(float(_x));
		unsigned int y1 = GetRawY(float(_y));
		if (x1>=widthx || y1>=widthy) return 8*default; 
		return (8.0f*(*data)[x1+y1*widthx]);
	}

	int IsInMap(float _x, float _y) {
		unsigned int x1 = GetRawX(_x);
		unsigned int y1 = GetRawY(_y);
		if (x1>=widthx || y1>=widthy) return 0;
		return 1;
	}

	int IsDefault(float _x, float _y) {
		unsigned int x1 = GetRawX(_x);
		unsigned int y1 = GetRawY(_y);
		if ((*data)[x1 + y1*widthx] - 0.5 < default)
			return 1;
		return 0;
	}

	void MakeDerivative(int _use16bit=0);

	llMap* Filter(unsigned long _dist, int _overwrite, llCommands *_batch);

	float GetDer1XMax(void) {
		return 8.f*x1max;
	}
	float GetDer1YMax(void) {
		return 8.f*y1max;
	}
	float GetDer2XMax(void) {
		return 8.f*x2max;
	}
	float GetDer2YMax(void) {
		return 8.f*y2max;
	}

	float GetDer1X(unsigned int _x, unsigned int _y) {
		//std::cout << "x1:" << _x << ":" << _y << ":" << (*data1x)[_x + _y*widthx] << std::endl;
		return 8.f*(*data1x)[_x + _y*widthx];
	}
	float GetDer1Y(unsigned int _x, unsigned int _y) {
		//std::cout << "y1:" << _x << ":" << _y << ":" << (*data1y)[_x + _y*widthx] << std::endl;
		return 8.f*(*data1y)[_x + _y*widthx];
	}
	float GetDer2X(unsigned int _x, unsigned int _y) {
		//std::cout << "x2:" << _x << ":" << _y << ":" << (*data2x)[_x + _y*widthx] << std::endl;
		return 8.f*(*data2x)[_x + _y*widthx];
	}
	float GetDer2Y(unsigned int _x, unsigned int _y) {
		//std::cout << "y2:" << _x << ":" << _y << ":" << (*data2y)[_x + _y*widthx] << std::endl;
		return 8.f*(*data2y)[_x + _y*widthx];
	}

	float GetRawDer1X(unsigned int _x, unsigned int _y) {
		return (*data1x)[_x + _y*widthx];
	}
	float GetRawDer1Y(unsigned int _x, unsigned int _y) {
		return (*data1y)[_x + _y*widthx];
	}
	float GetRawDer2X(unsigned int _x, unsigned int _y) {
		return (*data2x)[_x + _y*widthx];
	}
	float GetRawDer2Y(unsigned int _x, unsigned int _y) {
		return (*data2y)[_x + _y*widthx];
	}

	void  SetScaling(float _s){scaling=_s;};
	float GetScaling(){return scaling;};


};

#endif

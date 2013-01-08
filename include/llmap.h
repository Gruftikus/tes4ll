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
			for (int i=0; i<_size ; i++) {
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

		if (y <= 8192 && y >= -8192)
			val = float(y);
		else if (y <= 16383 && y >= -16383)
			val = float((y-mult*8192)*2 + mult*8192);
		else if (y <= 24575 && y >= -24575)
			val = float((y-mult*16384)*4 + mult*24576);
		else
			val = float((y-mult*24576)*8 + mult*57344);
		return val;
	}
	
};

class llMap {

protected:

	unsigned int  widthx,   widthy;
	unsigned int  rndx,     rndy,   rnd_widthx, rnd_widthy;
	unsigned int  rnd_x1, rnd_y1,   rnd_x2, rnd_y2;

	float         f_widthx, f_widthy;
	float         widthx_per_raw,  widthy_per_raw;
	float         widthx_per_raw2, widthy_per_raw2;

	float x1, y1, x2, y2, z, scaling;
	int   uneven;

	llShortarray *sdata;
	unsigned int *idata;
	float defaultheight;
	int makeshort;
	llLogger *mesg;


public:

	llMap(unsigned int _x = 0, unsigned int _y = 0, int _makeshort=0, float _default = 0);
	llMap(unsigned int _x,     unsigned int _y,  llShortarray *_data, float _default = 0);

	~llMap();
	llMap * Clone(int _expand, int _makeshort);

	//Raw:
	unsigned int GetWidthX(void) {return widthx;};
	unsigned int GetWidthY(void) {return widthy;};

	void SetElementRaw(unsigned int _x, unsigned int _y, float _val) {
		if (_x >= widthx || _y >= widthy) return;
		if (sdata) sdata->SetElement(_x+_y*widthx, _val);
		else       idata[_x+_y*widthx] = (unsigned int) _val;
	};
	void SetElementRaw(unsigned int _x, unsigned int _y, int _val) {
		if (_x >= widthx || _y >= widthy) return;
		if (sdata) sdata->SetElement(_x+_y*widthx, float(_val));
		else       idata[_x+_y*widthx] = (unsigned int) _val;
	};
	float GetElementRaw(unsigned int _x, unsigned int _y) {
		if (_x >= widthx || _y >= widthy) return defaultheight;
		if (sdata) return (*sdata)[_x+_y*widthx];
		return (float) idata[_x+_y*widthx];
	};
	void ChangeElementRaw(unsigned int _x, unsigned int _y, float _val) {
		if (_x >= widthx || _y >= widthy) return;
		if (sdata) sdata->SetElement(_x + _y*widthx, (*sdata)[_x+_y*widthx] + _val);
		else       idata[_x + _y*widthx] = unsigned int(float(idata[_x+_y*widthx]) + _val);
	};
	
	void SetEven() {uneven=0;};

	//With coordinates:
	void SetCoordSystem(float _x1, float _y1, float _x2, float _y2, float _z) {
		x1 = _x1;
		y1 = _y1;
		x2 = _x2;
		y2 = _y2;
		z  = _z;
		if (widthx > 1)
			widthx_per_raw = (x2 - x1) / (f_widthx - float(uneven));
		else
			widthx_per_raw = 0.f;
		if (widthy > 1)
			widthy_per_raw = (y2 - y1) / (f_widthy - float(uneven));
		else
			widthy_per_raw = 0.f;
		if (uneven) {
			widthx_per_raw2 =  widthx_per_raw / 2.0f;
			widthy_per_raw2 =  widthy_per_raw / 2.0f;
		} else {
			widthx_per_raw2 =  0.0f;
			widthy_per_raw2 =  0.0f;
		}
	}
	float GetX1() {return x1;};
	float GetY1() {return y1;};
	float GetX2() {return x2;};
	float GetY2() {return y2;};
	float GetZScale() {return z;};


	//Transformation:
	unsigned int GetRawX(float _x) {
		if (_x < x1) return 0;
		unsigned int xx = unsigned int((_x - x1 + widthx_per_raw2) / widthx_per_raw);
		return xx;
	}
	unsigned int GetRawY(float _y) {
		if (_y < y1) return 0;
		unsigned int yy = unsigned int((_y - y1 + widthy_per_raw2) / widthy_per_raw);
		return yy;
	}
	float GetCoordX(unsigned int _x) {
		return ((float)_x * widthx_per_raw + x1);
	}

	float GetCoordY(unsigned int _y) {
		return ((float)_y  * widthy_per_raw + y1);
	}
	float GetWidthXPerRaw() {
		return widthx_per_raw;
	};
	float GetWidthYPerRaw() {
		return widthy_per_raw;
	};

	//float GetZCoord(unsigned int _x, unsigned int _y) {
	float GetZ(unsigned int _x, unsigned int _y) {
		if (_x>=widthx || _y>=widthy) return z*defaultheight; 
		return z * GetElementRaw(_x, _y);
	}

	float GetZ(float _x, float _y) {
		unsigned int x1 = GetRawX(_x);
		unsigned int y1 = GetRawY(_y);
		if (x1>=widthx || y1>=widthy) return z*defaultheight; 
		return z * GetElementRaw(x1, y1);
	}

	float GetZ(double _x, double _y) {
		unsigned int x1 = GetRawX(float(_x));
		unsigned int y1 = GetRawY(float(_y));
		if (x1>=widthx || y1>=widthy) return z*defaultheight; 
		return z * GetElementRaw(x1, y1);
	}

	int IsInMap(float _x, float _y) {
		if (_x< x1 || _y < y1) return 0;
		unsigned int x1 = GetRawX(_x);
		unsigned int y1 = GetRawY(_y);
		if (x1>=widthx || y1>=widthy) return 0;
		return 1;
	}

	int IsDefault(float _x, float _y) {
		unsigned int x1 = GetRawX(_x);
		unsigned int y1 = GetRawY(_y);
		if (GetElementRaw(x1, y1) - 0.5 < defaultheight)
			return 1;
		return 0;
	}

	float GetDefaultHeight(void) {
		return defaultheight;
	}


	void  SetScaling(float _s){scaling=_s;};
	float GetScaling(){return scaling;};

	void InitRnd(unsigned int _x1, unsigned int _y1, unsigned int _x2, unsigned int _y2);

	unsigned int GetRndX();
	unsigned int GetRndY();

	float GetCoordRndX() {
		return GetCoordX(GetRndX());
	}
	float GetCoordRndY() {
		return GetCoordY(GetRndY());
	}

	int GetTupel(unsigned int _x, unsigned int _y, unsigned char *_x1, unsigned char *_x2, unsigned char *_x3, unsigned char *_x4) {
		if (_x>=widthx || _y>=widthy) return 0;
		if (idata) {
			*_x1 = char(idata[_x+_y*widthx] & 0xff);
			*_x2 = char(idata[_x+_y*widthx] & 0xff00     >> 8);
			*_x3 = char(idata[_x+_y*widthx] & 0xff0000   >> 16);
			*_x4 = char(idata[_x+_y*widthx] & 0xff000000 >> 24);
			return 1;
		}
		*_x1 = char( int(GetElementRaw(_x, _y)) & 0xff);
		*_x2 = char((int(GetElementRaw(_x, _y)) & 0xff00)     >> 8);
		*_x3 = char((int(GetElementRaw(_x, _y)) & 0xff0000)   >> 16);
		*_x4 = char((int(GetElementRaw(_x, _y)) & 0xff000000) >> 24);
		return 1;
	}

	int SetBlue(unsigned int _x, unsigned int _y, unsigned char _val) {
		if (_x>=widthx || _y>=widthy) return 0;
		if (idata) {
			idata[_x+_y*widthx] = (idata[_x+_y*widthx] & 0xffffff00) | _val;
			return 1;
		}
		SetElementRaw(_x, _y, float((int(GetElementRaw(_x, _y)) & 0xffffff00) | _val));
		return 1;
	}

	int SetGreen(unsigned int _x, unsigned int _y, unsigned char _val) {
		if (_x>=widthx || _y>=widthy) return 0;
		if (idata) {
			idata[_x+_y*widthx] = (idata[_x+_y*widthx] & 0xffff00ff) | (_val << 8);
			return 1;
		}
		SetElementRaw(_x, _y, float((int(GetElementRaw(_x, _y)) & 0xffff00ff) | (_val << 8)));
		return 1;
	}

	int SetRed(unsigned int _x, unsigned int _y, unsigned char _val) {
		if (_x>=widthx || _y>=widthy) return 0;
		if (idata) {
			idata[_x+_y*widthx] = (idata[_x+_y*widthx] & 0xff00ffff) | (_val << 16);
			return 1;
		}
		SetElementRaw(_x, _y, float((int(GetElementRaw(_x, _y)) & 0xff00ffff) | (_val << 16)));
		return 1;
	}

	int SetAlpha(unsigned int _x, unsigned int _y, unsigned char _val) {
		if (_x>=widthx || _y>=widthy) return 0;
		if (idata) {
			idata[_x+_y*widthx] = (idata[_x+_y*widthx] & 0x00ffffff) | (_val << 24);
			return 1;
		}
		SetElementRaw(_x, _y, float((int(GetElementRaw(_x, _y)) & 0x00ffffff) | (_val << 24)));
		return 1;
	}

};

#endif

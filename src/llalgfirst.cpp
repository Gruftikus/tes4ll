#include "..\include\llalgfirst.h"
#include <string.h>
#include <stdio.h>



//constructor
llAlgFirst::llAlgFirst(llMap *_map, float _x00, float _y00, float _x11, float _y11) :
llAlg( _map, _x00, _y00, _x11, _y11) {
	_map->MakeDerivative();
	loc_ceiling=0;
}

float llAlgFirst::GetCeiling(float *_ceiling) {
	if (_ceiling) {
		if (add)
			*_ceiling += loc_ceiling*add;
		if (multiply)
			*_ceiling *= loc_ceiling*multiply;
		return *_ceiling;
	} else {
		return loc_ceiling*multiply + add*loc_ceiling;
	}
}

float llAlgFirst::GetValue(float _x, float _y,float *_value) {

	float loc_value=0;
	int xx = heightmap->GetX(int(_x));
	int yy = heightmap->GetY(int(_y));

	if (_x>=x00 && _x<=x11 && _y>=y00 && _y<=y11) {
		loc_value=
			(fabs(float(heightmap->GetX1Coord(xx,yy))) + 
			fabs(float(heightmap->GetY1Coord(xx,yy))) );
	}

	if (loc_value>loc_ceiling) loc_ceiling=loc_value;

	if (_value) {
		if (add)
			*_value += loc_value*add;
		if (multiply)
			*_value *= loc_value*multiply;
		return *_value;
	} else {
		return loc_value*multiply + add*loc_value;
	}
}

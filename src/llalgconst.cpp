#include "..\include\llalgconst.h"
#include <string.h>
#include <stdio.h>



//constructor
llAlgConst::llAlgConst(llMap *_map, float _x00, float _y00, float _x11, float _y11) :
llAlg( _map, _x00, _y00, _x11, _y11) {
}

float llAlgConst::GetCeiling(float *_ceiling) {

	float loc_ceiling=1.;

	if (_ceiling) {
		if (add)
			*_ceiling += loc_ceiling*add;
		if (multiply)
			*_ceiling *= loc_ceiling*multiply;
		return *_ceiling;
	} else {
		return loc_ceiling*add*multiply;
	}
}

float llAlgConst::GetValue(float _x, float _y,float *_value) {
	float loc_value=1.;

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

#include "..\include\llAlgRadial.h"
#include <string.h>
#include <stdio.h>



//constructor
llAlgRadial::llAlgRadial(llMap *_map, float _x00, float _y00, float _x11, float _y11) :
llAlg( _map, _x00, _y00, _x11, _y11) {

	_map->MakeDerivative();
	loc_ceiling = 0;

	Near=0;
	Far=100000;
	ValueAtNear  = 0.5f;
	ValueAtFar   = 1.0f;
	X = 0.f;
	Y = 0.f;
}



float llAlgRadial::GetCeiling(float *_ceiling) {

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

float llAlgRadial::GetValue(float _x, float _y, float *_value) {

	float loc_value=ValueAtFar;

	float z = sqrt((_x-X)*(_x-X) + (_y-Y)*(_y-Y));

	if (z<Near) 
		loc_value=ValueAtNear;
	else if (z>Far) 
		loc_value=ValueAtFar;
	else 
		loc_value =  ValueAtNear + (ValueAtFar - ValueAtNear) * ( (z - Far) / (Far / Near));

	if (loc_value > loc_ceiling  && loc_value < ValueAtFar && loc_value < ValueAtNear) 
		loc_ceiling = loc_value;

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

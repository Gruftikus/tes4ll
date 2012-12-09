#include "..\include\llalgslope.h"
#include <string.h>
#include <stdio.h>



//constructor
llAlgSlope::llAlgSlope(llMap *_map, float _x00, float _y00, float _x11, float _y11) :
llAlg( _map, _x00, _y00, _x11, _y11) {

	_map->MakeDerivative();
	loc_ceiling=0;

	Lowest=-8000;
	Highest=8000;
	ValueAtLowest  = 0.2f;
	ValueAtHighest = 1.0f;
}



float llAlgSlope::GetCeiling(float *_ceiling) {

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

float llAlgSlope::GetValue(float _x, float _y, float *_value) {

	float loc_value=0;

	float z = float(heightmap->GetZ(_x, _y));

	if (z<Lowest) loc_value=ValueAtLowest;
	else if (z>Highest) loc_value=ValueAtHighest;
	else {
		loc_value= ValueAtLowest + 
			((z - Lowest)/(Highest-Lowest)) * (ValueAtHighest-ValueAtLowest);
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

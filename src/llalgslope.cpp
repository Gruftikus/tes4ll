#include "..\include\llalgslope.h"
#include <string.h>
#include <stdio.h>

//constructor
llAlgSlope::llAlgSlope(llAlgList *_alg_list, char *_map) : llAlg(_map) {

	alg_list = _alg_list;
	
	loc_ceiling = 0;

	lowest           = -8000;
	highest          =  8000;
	value_at_lowest  =  0.2f;
	value_at_highest =  1.0f;

	SetCommandName("AlgLinear");
}

int llAlgSlope::RegisterOptions(void) {
	if (!llAlg::RegisterOptions()) return 0;

	RegisterValue("-highest", &highest);
	RegisterValue("-lowest",  &lowest);
	RegisterValue("-minval",  &value_at_lowest);
	RegisterValue("-maxval",  &value_at_highest);	

	return 1;
}

double llAlgSlope::GetCeiling(double *_ceiling) {

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

double llAlgSlope::GetValue(float _x, float _y, double *_value) {

	double loc_value=0;

	float z = heightmap->GetZ(_x, _y);

	if (z < lowest) 
		loc_value = value_at_lowest;
	else if (z > highest) 
		loc_value = value_at_highest;
	else {
		loc_value= value_at_lowest + 
			((z - lowest)/(highest - lowest)) * (value_at_highest - value_at_lowest);
	}

	if (loc_value > loc_ceiling) 
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

int llAlgSlope::Init(void) {
	if (!llAlg::Init()) return 0;
	alg_list->AddAlg(this);
	heightmap->MakeDerivative();
	return 1;
}
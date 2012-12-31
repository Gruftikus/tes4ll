#include "..\include\llalgradial.h"
#include <string.h>
#include <stdio.h>



//constructor
llAlgRadial::llAlgRadial(llAlgList *_alg_list, char *_map) : llAlg(_map) {

	alg_list    = _alg_list;
	loc_ceiling = 0;

	my_near = 0;
	my_far  = 100000;
	value_at_near  = 0.5f;
	value_at_far   = 1.0f;
	x = 0.f;
	y = 0.f;

	SetCommandName("AlgRadial");
}

int llAlgRadial::RegisterOptions(void) {
	if (!llAlg::RegisterOptions()) return 0;

	RegisterValue("-near",   &my_near);
	RegisterValue("-far",    &my_far);
	RegisterValue("-minval", &value_at_near);
	RegisterValue("-maxval", &value_at_far);
	RegisterValue("-x",      &x);
	RegisterValue("-y",      &y);

	return 1;
}

double llAlgRadial::GetCeiling(double *_ceiling) {

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

double llAlgRadial::GetValue(float _x, float _y, double *_value) {

	float loc_value = value_at_far;

	float z = sqrt((_x-x)*(_x-x) + (_y-y)*(_y-y));

	if (z < my_near) 
		loc_value = value_at_near;
	else if (z > my_far) 
		loc_value = value_at_far;
	else 
		loc_value = value_at_near + (value_at_far - value_at_near) * ( (z - my_near) / (my_far - my_near));

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

int llAlgRadial::Init(void) {
	if (!llAlg::Init()) return 0;
	alg_list->AddAlg(this);
	return 1;
}
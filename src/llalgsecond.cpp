#include "..\include\llalgsecond.h"
#include <string.h>
#include <stdio.h>


//constructor
llAlgSecond::llAlgSecond(llAlgList *_alg_list, char *_map) : llAlg(_map) {
	alg_list    = _alg_list;
	loc_ceiling = 0;

	SetCommandName("AlgSecondOrder");
}

double llAlgSecond::GetCeiling(double *_ceiling) {

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

double llAlgSecond::GetValue(float _x, float _y, double *_value) {

	double loc_value = 0;
	unsigned int xx = heightmap->GetRawX(_x);
	unsigned int yy = heightmap->GetRawY(_y);

	if (_x>=x00 && _x<=x11 && _y>=y00 && _y<=y11) {
		loc_value =
			( fabs(heightmap->GetX2(xx, yy)) 
			+ fabs(heightmap->GetY2(xx, yy)) );
	}

	if (loc_value > loc_ceiling && loc_value < 10.f) 
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

int llAlgSecond::Init(void) {
	if (!llAlg::Init()) return 0;
	alg_list->AddAlg(this);
	heightmap->MakeDerivative();
	return 1;
}

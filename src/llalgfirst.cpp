#include "..\include\llalgfirst.h"
#include <string.h>
#include <stdio.h>



//constructor
llAlgFirst::llAlgFirst(llAlgList *_alg_list, char *_map) : llAlg(_map) {

	alg_list    = _alg_list;
	loc_ceiling = 0;

	SetCommandName("AlgFirstOrder");
}

double llAlgFirst::GetCeiling(double *_ceiling) {
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

double llAlgFirst::GetValue(float _x, float _y, double *_value) {

	double loc_value = 0;
	unsigned int xx = heightmap->GetRawX(_x);
	unsigned int yy = heightmap->GetRawY(_y);

	if (_x>=x00 && _x<=x11 && _y>=y00 && _y<=y11) {
		loc_value =
			( fabs(heightmap->GetX1(xx, yy)) 
			+ fabs(heightmap->GetY1(xx, yy)) );
	}

	if (loc_value > loc_ceiling && loc_value < 2.0f) //maxslope is 200%
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

int llAlgFirst::Init(void) {
	if (!llAlg::Init()) return 0;
	alg_list->AddAlg(this);
	heightmap->MakeDerivative();
	return 1;
}
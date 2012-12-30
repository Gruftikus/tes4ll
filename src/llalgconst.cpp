#include "..\include\llalgconst.h"

#include <string.h>
#include <stdio.h>



//constructor
llAlgConst::llAlgConst(llAlgList *_alg_list, char *_map) : llAlg(_map) {
	alg_list = _alg_list;
	SetCommandName("AlgConst");
}

double llAlgConst::GetCeiling(double *_ceiling) {

	double loc_ceiling=1.;

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

double llAlgConst::GetValue(float _x, float _y, double *_value) {

	double loc_value=1.;

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

int llAlgConst::Init(void) {
	if (!llAlg::Init()) return 0;
	alg_list->AddAlg(this);
	return 1;
}
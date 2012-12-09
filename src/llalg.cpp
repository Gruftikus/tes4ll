//base class for all algorithms

#include "..\include\llalg.h"
#include <string.h>
#include <stdio.h>

//constructor
llAlg::llAlg(llMap * _map, float _x00, float _y00, float _x11, float _y11) {
	heightmap = _map;
	x00 = _x00;
	y00 = _y00;
	x11 = _x11;
	y11 = _y11;
}


float llAlg::GetCeiling(float *_ceiling) {
	return 0;
}

float llAlg::GetValue(float _x, float _y,float *_value) {
	return 0;
}

int llAlg::Init(void) {
	return 0;
}

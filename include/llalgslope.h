#ifndef _PLLALGSLOPE_H_
#define _PLLALGSLOPE_H_

#include <iostream>
#include "../include/llmap.h"
#include "../include/llalglist.h"

class llAlgSlope : public llAlg {

private:

	float lowest, highest, value_at_lowest, value_at_highest;

	llAlgList *alg_list;

public:

	llAlgSlope(llAlgList *_alg_list, char *_map);

	double GetCeiling(double *_ceiling = NULL); 
	double GetValue(float _x, float _y, double *_value = NULL); 

	virtual llWorker * Clone() {
		return new llAlgSlope(*this);
	}

	int    Init(void);

};

#endif

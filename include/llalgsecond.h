#ifndef _PLLALGSECOND_H_
#define _PLLALGSECOND_H_

#include <iostream>
#include "../include/llmap.h"
#include "../include/llalglist.h"

class llAlgSecond : public llAlg {

private:

	llAlgList *alg_list;

public:

	//constructor
	llAlgSecond(llAlgList *_alg_list, char *_map);

	double GetCeiling(double *_ceiling = NULL); 
	double GetValue(float _x, float _y, double *_value = NULL); 

	virtual llWorker * Clone() {
		return new llAlgSecond(*this);
	}

	int    Init(void);
};

#endif

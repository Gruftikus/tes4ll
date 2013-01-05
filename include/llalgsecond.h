#ifndef _PLLALGSECOND_H_
#define _PLLALGSECOND_H_

#include <iostream>
#include "../include/llmap.h"
#include "../include/llalglist.h"

class llAlgSecond : public llAlg {

private:

	char *alg_list;
	char *sourcename;
	llMap *mapx2, *mapy2;

public:

	//constructor
	llAlgSecond(char *_alg_list, char *_map);

	double GetCeiling(double *_ceiling = NULL); 
	double GetValue(float _x, float _y, double *_value = NULL); 

	virtual llWorker * Clone() {
		return new llAlgSecond(*this);
	}

	int RegisterOptions(void);
	int Init(void);
};

#endif

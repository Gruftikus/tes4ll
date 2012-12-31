#ifndef _PLLALGRADIAL_H_
#define _PLLALGRADIAL_H_

#include "../include/llalglist.h"

class llAlgRadial : public llAlg {

private:

	llAlgList *alg_list;
	float my_near, my_far;
	float value_at_near, value_at_far, x, y;

public:


	//constructor
	llAlgRadial(llAlgList *_alg_list, char *_map);

    double GetCeiling(double *_ceiling = NULL); 
    double GetValue(float _x, float _y, double *_value = NULL); 

	virtual llWorker * Clone() {
		return new llAlgRadial(*this);
	}

	int RegisterOptions(void);
	int Init(void);

};

#endif

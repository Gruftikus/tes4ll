#ifndef _PLLALGFIRST_H_
#define _PLLALGFIRST_H_

#include <iostream>
#include "../include/llmap.h"
#include "../include/llalglist.h"

class llAlgFirst : public llAlg {

private:

	llAlgList *alg_list;

public:

	//constructor
	llAlgFirst(llAlgList *_alg_list, char *_map);

	double GetCeiling(double *ceiling = NULL); 
	double GetValue(float x, float y, double *value = NULL); 

	virtual llWorker * Clone() {
		return new llAlgFirst(*this);
	}

	int    Init(void);
};

#endif

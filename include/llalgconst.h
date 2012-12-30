#ifndef _PLLALGCONST_H_
#define _PLLALGCONST_H_

#include <iostream>
#include "../include/llmap.h"
#include "../include/llalglist.h"

class llAlgConst : public llAlg {

 private:

	llAlgList *alg_list;

 public:

    //constructor
    llAlgConst(llAlgList *_alg_list, char *_map);

    double GetCeiling(double *ceiling = NULL); 
    double GetValue(float x, float y, double *value = NULL); 

	virtual llWorker * Clone() {
		return new llAlgConst(*this);
	}

	int    Init(void);
};

#endif

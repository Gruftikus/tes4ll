#ifndef _PLLALGFIRST_H_
#define _PLLALGFIRST_H_

#include <iostream>
#include "../include/llmap.h"
#include "../include/llalglist.h"

class llAlgFirst : public llAlg {

private:

	char *alg_list;
	char *sourcename;
	llMap *mapx1, *mapy1;

public:

	//constructor
	llAlgFirst(char *_alg_list, char *_map);

	double GetCeiling(double *ceiling = NULL); 
	double GetValue(float x, float y, double *value = NULL); 

	virtual llWorker * Clone() {
		return new llAlgFirst(*this);
	}

	int RegisterOptions(void);
	int Init(void);
};

#endif

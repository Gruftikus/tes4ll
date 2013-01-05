#ifndef _PLLALGPEAKFINDER_H_
#define _PLLALGPEAKFINDER_H_

#include <iostream>
#include "../include/llmap.h"
#include "../include/llalglist.h"
#include "../include/llpointlist.h"

class llAlgPeakFinder : public llAlg {

private:

	llPointList * points;

	float radius, scan_radius, value_at_lowest, value_at_highest, lowest; 
	int linear;

	char *alg_list;

public:

	llAlgPeakFinder(char *_alg_list, char *_map);

	double GetCeiling(double *_ceiling = NULL); 
	double GetValue(float _x, float _y, double *_value = NULL); 

	virtual llWorker * Clone() {
		return new llAlgPeakFinder(*this);
	}

	int RegisterOptions(void);
	int Init(void);
};

#endif

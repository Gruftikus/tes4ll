#ifndef _PLLSETCONTOUR_H_
#define _PLLSETCONTOUR_H_

#include <iostream>
#include "../include/llset.h"

class llSetContour : public llSet {

private:

	float gridx, gridy, z, offsetx, offsety;
	int   findmin, findmax, linear, onlyintracell;
	int   setmin, setmax;

public:

	llSetContour();

	virtual llWorker * Clone() {
		return new llSetContour(*this);
	}

	int RegisterOptions(void);
	int Init(void);

};

#endif

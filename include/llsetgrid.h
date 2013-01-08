#ifndef _PLLSETGRID_H_
#define _PLLSETGRID_H_

#include <iostream>
#include "../include/llset.h"

class llSetGrid : public llSet {

private:

	float gridx, gridy;

public:

	llSetGrid();

	virtual llWorker * Clone() {
		return new llSetGrid(*this);
	}

	int RegisterOptions(void);
	int Init(void);

};

#endif

#ifndef _PLLSPLITATGRID_H_
#define _PLLSPLITATGRID_H_

#include <iostream>
#include "../include/llset.h"

class llSplitAtGrid : public llSet {

private:

	float gridx, gridy, zmin, max;

public:

	llSplitAtGrid();

	virtual llWorker * Clone() {
		return new llSplitAtGrid(*this);
	}

	int RegisterOptions(void);
	int Init(void);

};

#endif

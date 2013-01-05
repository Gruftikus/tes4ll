#ifndef _PLLMAKEDERIVATIVES_H_
#define _PLLMAKEDERIVATIVES_H_

#include <iostream>
#include "../include/llworker.h"
#include "../include/llmap.h"

class llMakeDerivatives : public llWorker {

protected:

	char *sourcename;
	int   makeshort;

	float x1max;
	float y1max;
	float x2max;
	float y2max;

public:

	llMakeDerivatives();

	llWorker * Clone() {
		return new llMakeDerivatives(*this);
	}

	virtual int RegisterOptions(void);
	virtual int Init(void);

};

#endif

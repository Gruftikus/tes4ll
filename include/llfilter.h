#ifndef _PLLFILTER_H_
#define _PLLFILTER_H_

#include <iostream>
#include "../include/llworker.h"
#include "../include/llmap.h"

class llFilter : public llWorker {

protected:

	char *sourcename, *targetname;
	int dist;
	int makeshort, overwrite, makederivatives;

public:

	llFilter();

	llWorker * Clone() {
		return new llFilter(*this);
	}

	virtual int RegisterOptions(void);
	virtual int Init(void);

};

#endif

#ifndef _PLLCREATEMAP_H_
#define _PLLCREATEMAP_H_

#include <iostream>
#include "../include/llworker.h"
#include "../include/llmap.h"

class llCreateMap : public llWorker {

protected:

	int widthx, widthy;
	float x1, y1, x2, y2, z;
	char *mapname;
	int   even;

public:

	llCreateMap();

	llWorker * Clone() {
		return new llCreateMap(*this);
	}

	virtual int RegisterOptions(void);
	virtual int Init(void);

};

#endif

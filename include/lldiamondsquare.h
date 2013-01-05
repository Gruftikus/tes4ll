#ifndef _PLLDIAMONDSQUARE_H_
#define _PLLDIAMONDSQUARE_H_

#include <iostream>
#include "../include/llworker.h"
#include "../include/llmap.h"

class llDiamondSquare : public llWorker {

protected:

	int   steps;
	char *mapname;
	float range, offset;
	char *alg_list;

public:

	llDiamondSquare();

	llWorker * Clone() {
		return new llDiamondSquare(*this);
	}

	virtual int RegisterOptions(void);
	virtual int Init(void);

};

#endif

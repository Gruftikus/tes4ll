#ifndef _PLLFILLCOLORMAP_H_
#define _PLLFILLCOLORMAP_H_

#include <iostream>
#include "../include/llworker.h"
#include "../include/llmap.h"

class llFillColorMap : public llWorker {

protected:

	char *mapname;
	char *alg_list_red, *alg_list_blue, *alg_list_green, *alg_list_alpha;

public:

	llFillColorMap();

	llWorker * Clone() {
		return new llFillColorMap(*this);
	}

	virtual int RegisterOptions(void);
	virtual int Init(void);

};

#endif

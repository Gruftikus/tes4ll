#ifndef _PLLEXPORTMAP_H_
#define _PLLEXPORTMAP_H_

#include <iostream>
#include "../include/llworker.h"
#include "../include/llmap.h"

class llExportMap : public llWorker {

protected:

	char *mapname;
	char *filename;
	int  bits;

public:

	llExportMap();

	llWorker * Clone() {
		return new llExportMap(*this);
	}

	virtual int RegisterOptions(void);
	virtual int Init(void);

};

#endif

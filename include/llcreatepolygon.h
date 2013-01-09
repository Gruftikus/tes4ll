#ifndef _PLLCREATEPOLYGON_H_
#define _PLLCREATEPOLYGON_H_

#include <iostream>
#include "../include/llset.h"

class llCreatePolygon : public llSet {

private:

	float x1, y1, x2, y2;
	char *polygon_name;

public:

	llCreatePolygon();

	virtual llWorker * Clone() {
		return new llCreatePolygon(*this);
	}

	int RegisterOptions(void);
	int Init(void);

};

#endif

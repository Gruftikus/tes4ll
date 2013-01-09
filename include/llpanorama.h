#ifndef _PLLPANORAMA_H_
#define _PLLPANORAMA_H_

#include <iostream>
#include "../include/llset.h"

class llPanorama : public llSet {

private:

	float gx, gy, keepout;

public:

	llPanorama();

	virtual llWorker * Clone() {
		return new llPanorama(*this);
	}

	int RegisterOptions(void);
	int Init(void);

};

#endif

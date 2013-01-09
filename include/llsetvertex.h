#ifndef _PLLSETVERTEX_H_
#define _PLLSETVERTEX_H_

#include <iostream>
#include "../include/llset.h"

class llSetVertex : public llSet {

private:

	float x, y;

public:

	llSetVertex();

	virtual llWorker * Clone() {
		return new llSetVertex(*this);
	}

	int RegisterOptions(void);
	int Init(void);

};

#endif

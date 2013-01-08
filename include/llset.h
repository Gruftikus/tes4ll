#ifndef _PLLSET_H_
#define _PLLSET_H_

#include <iostream>
#include "../include/llworker.h"
#include "../include/llmaplist.h"

class llSet : public llWorker {

protected:

    llMap       *heightmap;
	char        *map;
	llPointList *points;

public:

    
    //constructor
    llSet();

	virtual llWorker * Clone() {
		std::cout << "I should never be here...." << std::endl;
		return new llWorker(*this);
	}

	virtual int RegisterOptions(void);
	virtual int Init(void);

};

#endif

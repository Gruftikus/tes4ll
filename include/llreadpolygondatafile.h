#ifndef _PLLREADPOLYGONDATAFILE_H_
#define _PLLREADPOLYGONDATAFILE_H_

#include <iostream>
#include "../include/llset.h"

class llReadPolygonDataFile : public llSet {

private:

	char *polygon_name, *filename;

public:

	llReadPolygonDataFile();

	virtual llWorker * Clone() {
		return new llReadPolygonDataFile(*this);
	}

	int RegisterOptions(void);
	int Init(void);

};

#endif

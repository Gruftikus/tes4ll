#ifndef _PLLALG_H_
#define _PLLALG_H_

#include <iostream>
#include "../include/llworker.h"
#include "../include/llmap.h"

class llAlg : public llWorker {

 protected:

    llMap *heightmap;
	char  *map;

    float x00, y00, x11, y11; //focus

    double loc_ceiling;

 public:

    float add, multiply;

    //constructor
    llAlg(char *_map);

    virtual double GetCeiling(double *_ceiling=NULL) = 0; 
	virtual double GetValue(float _x, float _y, double *_value=NULL) = 0; 

	virtual llWorker * Clone() {
		std::cout << "I should never be here...." << std::endl;
		return new llWorker(*this);
	}

	virtual int Init(void);

};

#endif

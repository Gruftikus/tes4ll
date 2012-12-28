#ifndef _PLLALGPEAKFINDER_H_
#define _PLLALGPEAKFINDER_H_

#include <iostream>
#include "../include/llmap.h"
#include "../include/llalg.h"
#include "../include/llpointlist.h"

class llAlgPeakFinder : public llAlg {

 private:

    llPointList * points;

 public:

    float Radius, Scanradius, ValueAtLowest, ValueAtHighest, Lowest; 
	int linear;
    //constructor
    llAlgPeakFinder(llMap *_map, float _x00, float _y00, float _x11, float _y11);

    float GetCeiling(float *_ceiling=NULL); 
    float GetValue(float _x, float _y,float *_value=NULL); 
    int   Init(void);
};

#endif

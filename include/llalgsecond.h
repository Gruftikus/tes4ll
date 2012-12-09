#ifndef _PLLALGSECOND_H_
#define _PLLALGSECOND_H_

#include <iostream>
#include "../include/llmap.h"
#include "../include/llalg.h"

class llAlgSecond : public llAlg {

 private:

 public:

    //constructor
    llAlgSecond(llMap *_map, float _x00, float _y00, float _x11, float _y11);

    float GetCeiling(float *_ceiling=NULL); 
    float GetValue(float _x, float _y, float *_value=NULL); 
};

#endif

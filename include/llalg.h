#ifndef _PLLALG_H_
#define _PLLALG_H_

#include <iostream>
#include "../include/llmap.h"

class llAlg {

 protected:

    llMap * heightmap;

    float x00,y00,x11,y11; //focus

    float loc_ceiling;

 public:

    float add,multiply;

    //constructor
    llAlg(llMap * _map, float _x00, float _y00, float _x11, float _y11);

    virtual float GetCeiling(float *_ceiling=NULL); 
    virtual float GetValue(float _x, float _y,float *_value=NULL); 

    virtual int Init(void);

};

#endif

#ifndef _PLLALGFIRST_H_
#define _PLLALGFIRST_H_

#include <iostream>
#include "../include/llmap.h"
#include "../include/llalg.h"

class llAlgFirst : public llAlg {

 private:

 public:

    //constructor
    llAlgFirst(llMap *_map, float _x00, float _y00, float _x11, float _y11);

    float GetCeiling(float *ceiling=NULL); 
    float GetValue(float x, float y,float *value=NULL); 
};

#endif

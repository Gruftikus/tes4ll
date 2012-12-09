#include "..\include\llalgpeakfinder.h"
#include <string.h>
#include <stdio.h>

//constructor
llAlgPeakFinder::llAlgPeakFinder(llMap *_map, float _x00, float _y00, float _x11, float _y11) :
llAlg( _map, _x00, _y00, _x11, _y11) {

	loc_ceiling=0;

	Radius=4096;
	Scanradius=8192;
	ValueAtLowest=0.2f;
	ValueAtHighest=1.0f;

	points = new llPointList(100,NULL);

};

int llAlgPeakFinder::Init(void) {

	int numfound=0;

	//let us scan over the heightmap 
	for (float x = x00+Scanradius; x< x11-Scanradius; x+=256) {
		for (float y = y00+Scanradius; y<y11-Scanradius; y+=256) {

			if (points->GetMinDistance(x,y) > Scanradius) {
				int is_highest=1;
				float z=heightmap->GetZ(x,y);

				//check for distance in pointlist

				//is this point the highest point?
				for (float x1 = 0; x1< Scanradius; x1 +=256) {
					for (float y1 = 0; y1< Scanradius; y1 +=256) {
						//is this point the highest point?
						if (heightmap->GetZ(x+x1,y+y1)>z || heightmap->GetZ(x-x1,y+y1)>z ||
							heightmap->GetZ(x+x1,y-y1)>z || heightmap->GetZ(x-x1,y-y1)>z) {
								is_highest=0;
								break;
						}
					}
					if (is_highest==0) break;
				}

				if (is_highest) {
					points->AddPoint(x,y,z);
					numfound++;
				}
			}
		}   
	}

	//	std::cout << "[Info] AlgPeakFinder identified " << numfound << " peaks" << std::endl;

	return 1;
}



float llAlgPeakFinder::GetCeiling(float *_ceiling) {

	if (_ceiling) {
		if (add)
			*_ceiling += loc_ceiling*add;
		if (multiply)
			*_ceiling *= loc_ceiling*multiply;
		return *_ceiling;
	} else {
		return loc_ceiling*multiply + add*loc_ceiling;
	}
}

float llAlgPeakFinder::GetValue(float _x, float _y, float *_value) {

	float loc_value=ValueAtLowest;

	float z = float(heightmap->GetZ(_x, _y));

	if (points->GetMinDistance(_x, _y) < Radius && z > Lowest) {
		loc_value=ValueAtHighest; 
		if (linear) 
			loc_value=ValueAtLowest + ((Radius-points->GetMinDistance(_x, _y))/Radius)*(ValueAtHighest-ValueAtLowest);
	}

	if (loc_value>loc_ceiling) loc_ceiling=loc_value;

	if (_value) {
		if (add)
			*_value += loc_value*add;
		if (multiply)
			*_value *= loc_value*multiply;
		return *_value;
	} else {
		return loc_value*multiply + add*loc_value;
	}
}

#include "..\include\llalgpeakfinder.h"
#include <string.h>
#include <stdio.h>

//constructor
llAlgPeakFinder::llAlgPeakFinder(char *_alg_list, char *_map) : llAlg(_map) {

	alg_list = _alg_list;

	loc_ceiling = 0;

	radius           = 4096.f;
	scan_radius      = 8192.f;
	value_at_lowest  = 0.2f;
	value_at_highest = 1.0f;
	linear           = 0;
	lowest           = -2000.f;

	points = new llPointList(100, NULL);

	SetCommandName("AlgPeakFinder");

};

int llAlgPeakFinder::RegisterOptions(void) {
	if (!llAlg::RegisterOptions()) return 0;

	RegisterFlag( "-linear",     &linear);
	RegisterValue("-lowest",     &lowest);
	RegisterValue("-minval",     &value_at_lowest);
	RegisterValue("-maxval",     &value_at_highest);
	RegisterValue("-radius",     &radius);
	RegisterValue("-scanradius", &scan_radius);

	return 1;
}

int llAlgPeakFinder::Init(void) {
	if (!llAlg::Init()) return 0;

	if (alg_list) {
		llAlgCollection *algs = _llAlgList()->GetAlgCollection(alg_list);
		if (!algs) {
			_llLogger()->WriteNextLine(-LOG_FATAL, "%s: alg collection [%s] not found", command_name, alg_list);
			return 0;
		}
		algs->AddAlg(this);
	}

	int stepsize = 1024;
	int numfound = 0;

	//let us scan over the heightmap 
	for (float x = x00+scan_radius; x < x11-scan_radius; x+=stepsize) {
		for (float y = y00+scan_radius; y < y11-scan_radius; y+=stepsize) {

			if (points->GetMinDistance(x,y) > scan_radius) {
				int is_highest = 1;
				float z = heightmap->GetZ(x,y);

				//check for distance in pointlist

				//is this point the highest point?
				for (float x1 = 0; x1 < scan_radius; x1 +=stepsize) {
					for (float y1 = 0; y1 < scan_radius; y1 +=stepsize) {
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

	_llLogger()->WriteNextLine(-LOG_INFO, "AlgPeakFinder identified %i peaks", numfound);

	return 1;
}



double llAlgPeakFinder::GetCeiling(double *_ceiling) {

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

double llAlgPeakFinder::GetValue(float _x, float _y, double *_value) {

	double loc_value = value_at_lowest;

	double z = double(heightmap->GetZ(_x, _y));

	if (points->GetMinDistance(_x, _y) < radius && z > lowest) {
		loc_value = value_at_highest; 
		if (linear) 
			loc_value = value_at_lowest + 
			((radius - points->GetMinDistance(_x, _y))/radius)*
			(value_at_highest - value_at_lowest);
	}

	if (loc_value > loc_ceiling) 
		loc_ceiling = loc_value;

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

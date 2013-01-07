
#include "..\include\llalgsecond.h"
#include "..\include\llmaplist.h"

#include <string.h>
#include <stdio.h>


//constructor
llAlgSecond::llAlgSecond(char *_alg_list, char *_map) : llAlg(_map) {
	alg_list    = _alg_list;
	loc_ceiling = 0;

	SetCommandName("AlgSecondOrder");
}

int llAlgSecond::RegisterOptions(void) {
	if (!llAlg::RegisterOptions()) return 0;

	RegisterValue("-map", &sourcename);
	RegisterValue("-alg", &alg_list);

	return 1;
}

double llAlgSecond::GetCeiling(double *_ceiling) {

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

double llAlgSecond::GetValue(float _x, float _y, double *_value) {

	if (!mapx2 || !mapy2) return 0.;

	double loc_value = 0;
	unsigned int xx = heightmap->GetRawX(_x);
	unsigned int yy = heightmap->GetRawY(_y);

	if (_x>=x00 && _x<=x11 && _y>=y00 && _y<=y11) {
		loc_value =
			( fabs(mapx2->GetZ(xx, yy)) 
			+ fabs(mapy2->GetZ(xx, yy)) );
	}

	if (loc_value > loc_ceiling && loc_value < 4.f)  //max turn about 400%
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

int llAlgSecond::Init(void) {
	if (Used("-map"))
		map = sourcename;

	if (!llAlg::Init()) return 0;

	if (alg_list) {
		llAlgCollection *algs = _llAlgList()->GetAlgCollection(alg_list);
		if (!algs) {
			_llLogger()->WriteNextLine(-LOG_FATAL, "%s: alg collection [%s] not found", command_name, alg_list);
			return 0;
		}
		algs->AddAlg(this);
	}

	if (!Used("-map"))
		sourcename = map;

	char * namex2 = new char[strlen(sourcename)+5];
	sprintf_s(namex2, strlen(sourcename)+5, "%s_d2x", sourcename);
	mapx2 = _llMapList()->GetMap(namex2);
	if (!mapx2) {
		_llLogger()->WriteNextLine(-LOG_WARNING,"%s: derivative map %s not existing", command_name, namex2);
		return 0;
	}
	char * namey2 = new char[strlen(sourcename)+5];
	sprintf_s(namey2, strlen(sourcename)+5, "%s_d1y", sourcename);
	mapy2 = _llMapList()->GetMap(namey2);
	if (!mapy2) {
		_llLogger()->WriteNextLine(-LOG_WARNING,"%s: derivative map %s not existing", command_name, namey2);
		return 0;
	}

	return 1;
}

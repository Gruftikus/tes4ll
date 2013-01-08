#include "..\include\llsetgridborder.h"
#include <string.h>
#include <stdio.h>

//constructor
llSetGridBorder::llSetGridBorder() : llSet() {

	SetCommandName("SetGridBorder");
}

int llSetGridBorder::RegisterOptions(void) {
	if (!llSet::RegisterOptions()) return 0;

	RegisterValue("-x",    &gridx, LLWORKER_OBL_OPTION);
	RegisterValue("-y",    &gridy, LLWORKER_OBL_OPTION);
	RegisterValue("-zmin", &zmin);

	return 1;
}


int llSetGridBorder::Init(void) {
	if (!llSet::Init()) return 0;
	float minab=256;

	int zused = 0;
	if (!Used("-zmin")) zused = 1;

	for (float x=floor(_llUtils()->x00/gridx)*gridx; x<=(_llUtils()->x11); x+=gridx) {
		float x1=x;
		if (x >= _llUtils()->x11) x1 = _llUtils()->x11;
		if (points->GetMinDistance(x1, _llUtils()->y00) > minab && (heightmap->GetZ(x1,_llUtils()->y00) > zmin || zused)) {
			points->AddPoint(x1, _llUtils()->y00, heightmap->GetZ(x1,_llUtils()->y00));	
			//gen_npoints++;
		}
		if (points->GetMinDistance(x1, _llUtils()->y11) > minab && (heightmap->GetZ(x1,_llUtils()->y11) > zmin || zused)) {
			points->AddPoint(x1, _llUtils()->y11, heightmap->GetZ(x1,_llUtils()->y11));	
			//gen_npoints++;
		}
	}
	for (float y=floor(_llUtils()->y00/gridy)*gridy; y<=(_llUtils()->y11); y+=gridy) {
		float y1=y;
		if (y >= _llUtils()->y11) y1 = _llUtils()->y11;
		if (points->GetMinDistance(_llUtils()->x00,y1) > minab && (heightmap->GetZ(_llUtils()->x00,y1) > zmin || zused)) {
			points->AddPoint(_llUtils()->x00, y1, heightmap->GetZ(_llUtils()->x00,y1));	
			//gen_npoints++;
		}
		if (points->GetMinDistance(_llUtils()->x11,y1) > minab && (heightmap->GetZ(_llUtils()->x11,y1) > zmin || zused)) {
			points->AddPoint(_llUtils()->x11, y1, heightmap->GetZ(_llUtils()->y11,y1));	
			//gen_npoints++;
		}
	}

	return 1;
}
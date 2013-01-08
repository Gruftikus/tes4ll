#include "..\include\llsetgrid.h"
#include <string.h>
#include <stdio.h>

//constructor
llSetGrid::llSetGrid() : llSet() {

	SetCommandName("SetGrid");
}

int llSetGrid::RegisterOptions(void) {
	if (!llSet::RegisterOptions()) return 0;

	RegisterValue("-x", &gridx, LLWORKER_OBL_OPTION);
	RegisterValue("-y", &gridy, LLWORKER_OBL_OPTION);

	return 1;
}


int llSetGrid::Init(void) {
	if (!llSet::Init()) return 0;

	for (float x=floor(_llUtils()->x00/gridx)*gridx; x<=(_llUtils()->x11+1); x+=gridx) {
		for (float y=floor(_llUtils()->y00/gridy)*gridy;y<=(_llUtils()->y11+1);y+=gridy) {
			float x1=x, y1=y;
			if (x >= _llUtils()->x11) x1 = _llUtils()->x11;
			if (y >= _llUtils()->y11) y1 = _llUtils()->y11;

			if (heightmap->IsInMap(x1,y1)) { //&& points->GetMinDistance(x1,y1) > minab) {
				points->AddPoint(x1,y1,heightmap->GetZ(x1,y1));	
				//gen_npoints++;
			}
		}
	}

	return 1;
}
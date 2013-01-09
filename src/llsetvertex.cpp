#include "..\include\llsetvertex.h"
#include <string.h>
#include <stdio.h>

//constructor
llSetVertex::llSetVertex() : llSet() {

	SetCommandName("SetVertex");
}

int llSetVertex::RegisterOptions(void) {
	if (!llSet::RegisterOptions()) return 0;

	RegisterValue("-x", &x, LLWORKER_OBL_OPTION);
	RegisterValue("-y", &y, LLWORKER_OBL_OPTION);

	return 1;
}


int llSetVertex::Init(void) {
	if (!llSet::Init()) return 0;

	if (!heightmap->IsInMap(x, y)) {
		_llLogger()->WriteNextLine(LOG_ERROR,"Point (%f, %f) not in map", x, y);
	} else {
		points->AddPoint(x, y, heightmap->GetZ(x, y));	
	}

	return 1;
}
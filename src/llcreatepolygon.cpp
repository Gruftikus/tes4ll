#include "..\include\llcreatepolygon.h"
#include <string.h>
#include <stdio.h>

//constructor
llCreatePolygon::llCreatePolygon() : llSet() {

	SetCommandName("CreatePolygon");

}

int llCreatePolygon::RegisterOptions(void) {
	if (!llSet::RegisterOptions()) return 0;

	RegisterValue("-x1",   &x1,           LLWORKER_OBL_OPTION);
	RegisterValue("-y1",   &y1,           LLWORKER_OBL_OPTION);
	RegisterValue("-x2",   &x1,           LLWORKER_OBL_OPTION);
	RegisterValue("-y2",   &y1,           LLWORKER_OBL_OPTION);
	RegisterValue("-name", &polygon_name, LLWORKER_OBL_OPTION);

	return 1;
}


int llCreatePolygon::Init(void) {
	if (!llSet::Init()) return 0;

	llPolygonList *polygons = _llMapList()->GetPolygonList(map);
	if (!polygons) {
		_llLogger()->WriteNextLine(-LOG_FATAL, "%s: no polygon list in map [%s]", command_name, map);
		return 0;
	}

	return polygons->AddPolygon(x1, y1, x2, y2, polygon_name);    

}
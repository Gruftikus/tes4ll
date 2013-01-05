#include "..\include\llcreatemap.h"
#include "..\include\llmaplist.h"

//constructor
llCreateMap::llCreateMap() : llWorker() {

	SetCommandName("CreateMap");
	mapname = NULL;
	z       = 1.0;

}

int llCreateMap::RegisterOptions(void) {
	if (!llWorker::RegisterOptions()) return 0;

	RegisterValue("-widthx",  &widthx, LLWORKER_OBL_OPTION);
	RegisterValue("-widthy",  &widthy, LLWORKER_OBL_OPTION);
	RegisterValue("-x1",      &x1,     LLWORKER_OBL_OPTION);
	RegisterValue("-y1",      &y1,     LLWORKER_OBL_OPTION);
	RegisterValue("-x2",      &x2,     LLWORKER_OBL_OPTION);
	RegisterValue("-y2",      &y2,     LLWORKER_OBL_OPTION);
	RegisterValue("-zscale",  &z);
	RegisterValue("-mapname", &mapname);

	return 1;
}

int llCreateMap::Init(void) {
	if (!llWorker::Init()) return 0;

	llMap * heightmap = new llMap(widthx, widthy);
	heightmap->SetCoordSystem(x1, y1, x2, y2, z);

	_llUtils()->x00 = x1;
	_llUtils()->y00 = y1;
	_llUtils()->x11 = x2;
	_llUtils()->y11 = y2;

	llPointList    *points     = new llPointList(0, NULL); //BUGBUG: quad list should be taken from global place
	llPolygonList  *polygons   = new llPolygonList(points, heightmap);
	llTriangleList *triangles  = new llTriangleList(0, points);
		
	if (!Used("-mapname"))
		_llMapList()->AddMap("_heightmap", heightmap, points, triangles, polygons);
	else
		_llMapList()->AddMap(mapname, heightmap, points, triangles, polygons);

	return 1;
}

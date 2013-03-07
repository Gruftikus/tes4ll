#include "../include/llimportmapfrommodlist.h"
#include "../../lltool/include/llmaplist.h"

llImportMapFromModlist::llImportMapFromModlist() : llWorker() {
	SetCommandName("ImportMapFromModlist");
	tes4qlod = NULL;
}

int llImportMapFromModlist::Prepare(void) {
	if (!llWorker::Prepare()) return 0;

	mapname  = NULL;

	return 1;
}

int llImportMapFromModlist::RegisterOptions(void) {
	if (!llWorker::RegisterOptions()) return 0;

	RegisterValue("-name", &mapname);

	return 1;
}

int llImportMapFromModlist::Exec(void) {
	if (!llWorker::Exec()) return 0;

	if (!Used("-name")) mapname = "_heightmap";

	if (_llMapList()->GetMap(mapname)) {
		_llLogger()->WriteNextLine(-LOG_INFO, "Delete old heightmap");
		_llMapList()->DeleteMap(mapname);
	}

	if (tes4qlod) delete tes4qlod;
	tes4qlod = new TES4qLOD();
	tes4qlod->RegisterOptions();
	tes4qlod->CheckFlag("-x");
	tes4qlod->CheckFlag("-M");
	tes4qlod->Prepare();
	tes4qlod->ReplaceFlags();

	
	if (!tes4qlod->Exec()) {
		delete tes4qlod;
		return 0;
	}


#if 0
	llMap * heightmap = new llMap(widthx, widthy);
	if (even) heightmap->SetEven();
	heightmap->SetCoordSystem(x1, y1, x2, y2, z);

	_llUtils()->x00 = x1;
	_llUtils()->y00 = y1;
	_llUtils()->x11 = x2;
	_llUtils()->y11 = y2;

	llQuadList     *quads      = heightmap->GenerateQuadList();
	llPointList    *points     = new llPointList(0, quads); 
	llPolygonList  *polygons   = new llPolygonList(points, heightmap);
	llTriangleList *triangles  = new llTriangleList(0, points);
		
	if (!Used("-name"))
		_llMapList()->AddMap("_heightmap", heightmap, points, triangles, polygons);
	else
		_llMapList()->AddMap(mapname, heightmap, points, triangles, polygons);
#endif
	return 1;
}

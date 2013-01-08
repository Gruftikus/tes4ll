//base class for all workers which sets vertex points

#include "..\include\llset.h"
#include "..\include\llmaplist.h"
#include <string.h>
#include <stdio.h>

//constructor
llSet::llSet() : llWorker() {
	map = NULL;
}

int llSet::RegisterOptions(void) {
	if (!llWorker::RegisterOptions()) return 0;

	RegisterValue("-map",  &map);

	return 1;
}

int llSet::Init(void) {
	llWorker::Init();

	if (!Used("-map"))
		map = "_heightmap";

	//get the corresponding map from the global map container
	heightmap = _llMapList()->GetMap(map);
	if (!heightmap) {
		_llLogger()->WriteNextLine(-LOG_FATAL, "%s: map [%s] not found", command_name, map);
		return 0;
	}

	llTriangleList *triangles = _llMapList()->GetTriangleList(map);
	if (triangles && triangles->GetN()) {
		_llLogger()->WriteNextLine(-LOG_FATAL, "%s: map [%s] already triangulated", command_name, map);
		return 0;
	}

	points = _llMapList()->GetPointList(map);
	if (!points) {
		_llLogger()->WriteNextLine(-LOG_FATAL, "%s: no vertex points in map [%s]", command_name, map);
		return 0;
	}

	return 1;
}

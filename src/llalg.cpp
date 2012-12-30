//base class for all algorithms

#include "..\include\llalg.h"
#include "..\include\llmaplist.h"
#include <string.h>
#include <stdio.h>

//constructor
llAlg::llAlg(char *_map) : llWorker() {

	map = _map;

	RegisterValue("-multiply", &multiply);
	RegisterValue("-add",      &add);

}


int llAlg::Init(void) {
	llWorker::Init();

	//get the corresponding map from the global map container
	if (map) {
		heightmap = _llMapList()->GetMap(map);
		if (!heightmap) {
			_llLogger()->WriteNextLine(-LOG_FATAL, "%s: map [%s] not found", command_name, map);
		}
	}

	if (!Used("-add") && !Used("-multiply")) {
		_llLogger()->WriteNextLine(-LOG_WARNING,"%s: no add or multiply factor specified (assuming -multiply=1)", GetCommandName());
		multiply = 1;
	} else if (!Used("-add")) {
		add = 0;
	} else if (!Used("-multiply")) {
		multiply = 0;
	}

	//get the focus from the global utility container
	x00 = _llUtils()->x00;
	y00 = _llUtils()->y00;
	x11 = _llUtils()->x11;
	y11 = _llUtils()->y11;

	return 1;
}

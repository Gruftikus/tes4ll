#include "..\include\llfillcolormap.h"
#include "..\include\llmaplist.h"
#include "..\include\llalglist.h"

//constructor
llFillColorMap::llFillColorMap() : llWorker() {

	SetCommandName("FillColorMap");
	mapname        = NULL;
	alg_list_blue  = NULL;
	alg_list_red   = NULL;
	alg_list_green = NULL;
	alg_list_alpha = NULL;
}

int llFillColorMap::RegisterOptions(void) {
	if (!llWorker::RegisterOptions()) return 0;

	RegisterValue("-map",       &mapname,        LLWORKER_OBL_OPTION);
	RegisterValue("-algblue",   &alg_list_blue);
	RegisterValue("-algred",    &alg_list_red);
	RegisterValue("-alggreen",  &alg_list_green);
	RegisterValue("-algalpha",  &alg_list_alpha);
	
	return 1;
}

int llFillColorMap::Init(void) {
	if (!llWorker::Init()) return 0;

	llAlgCollection *algs_blue = NULL;
	if (Used("-algblue")) {
		algs_blue = _llAlgList()->GetAlgCollection(alg_list_blue);
		if (!algs_blue) {
			_llLogger()->WriteNextLine(-LOG_ERROR, "%s: alg collection '%s' not found for blue channel", command_name, alg_list_blue);
			return 0;
		}
	}
	llAlgCollection *algs_red = NULL;
	if (Used("-algred")) {
		algs_red = _llAlgList()->GetAlgCollection(alg_list_red);
		if (!algs_red) {
			_llLogger()->WriteNextLine(-LOG_ERROR, "%s: alg collection '%s' not found for red channel", command_name, alg_list_red);
			return 0;
		}
	}
	llAlgCollection *algs_green = NULL;
	if (Used("-alggreen")) {
		algs_green = _llAlgList()->GetAlgCollection(alg_list_green);
		if (!algs_green) {
			_llLogger()->WriteNextLine(-LOG_ERROR, "%s: alg collection '%s' not found for green channel", command_name, alg_list_green);
			return 0;
		}
	}
	llAlgCollection *algs_alpha = NULL;
	if (Used("-algalpha")) {
		algs_alpha = _llAlgList()->GetAlgCollection(alg_list_alpha);
		if (!algs_alpha) {
			_llLogger()->WriteNextLine(-LOG_ERROR, "%s: alg collection '%s' not found for alpha channel", command_name, alg_list_alpha);
			return 0;
		}
	}

	llMap *map = _llMapList()->GetMap(mapname);
	if (!map) {
		_llLogger()->WriteNextLine(-LOG_ERROR,"%s: map '%s' not found", command_name, mapname);
		return 0;
	}

	unsigned int widthx = map->GetWidthX();
	unsigned int widthy = map->GetWidthY();

	for (unsigned int x=0; x<widthx; x++) {
		for (unsigned int y=0; y<widthy; y++) {
			if (algs_blue) {
				unsigned char blue = unsigned char(255*algs_blue->GetValue(map->GetCoordX(x), map->GetCoordX(y)));
				map->SetBlue(x, y, blue);
			}
			if (algs_red) {
				unsigned char red = unsigned char(255*algs_red->GetValue(map->GetCoordX(x), map->GetCoordX(y)));
				//std::cout << (unsigned int) red << std::endl;
				map->SetRed(x, y, red);
			}
			if (algs_green) {
				unsigned char green = unsigned char(255*algs_green->GetValue(map->GetCoordX(x), map->GetCoordX(y)));
				map->SetGreen(x, y, green);
			}
			if (algs_alpha) {
				unsigned char alpha = unsigned char(255*algs_alpha->GetValue(map->GetCoordX(x), map->GetCoordX(y)));
				map->SetAlpha(x, y, alpha);
			}
		}
	}

	return 1;
}

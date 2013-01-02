#include "..\include\lldiamondsquare.h"
#include "..\include\llmaplist.h"

//constructor
llDiamondSquare::llDiamondSquare() : llWorker() {

	SetCommandName("DiamondSquare");
	mapname = NULL;
	steps = 1;

}

int llDiamondSquare::RegisterOptions(void) {
	if (!llWorker::RegisterOptions()) return 0;

	RegisterValue("-steps",   &steps);
	RegisterValue("-mapname", &mapname);
	RegisterValue("-range",   &range, LLWORKER_OBL_OPTION);

	return 1;
}

int llDiamondSquare::Init(void) {
	if (!llWorker::Init()) return 0;

	llMap *oldmap = NULL;
	if (!Used("-mapname"))
		mapname = "_heightmap";

	oldmap = _llMapList()->GetMap(mapname);

	if (!oldmap) {
		_llLogger()->WriteNextLine(-LOG_ERROR,"%s: map %s not found", command_name, mapname);
		return 0;
	}

	unsigned int old_widthx = oldmap->GetWidthX();
	unsigned int old_widthy = oldmap->GetWidthY();

	unsigned int new_widthx = (old_widthx - 1) * 2 + 1;
	unsigned int new_widthy = (old_widthy - 1) * 2 + 1;

	llMap * heightmap = new llMap(new_widthx, new_widthy);
	heightmap->SetCoordSystem(oldmap->GetX1(), oldmap->GetY1(), oldmap->GetX2(), oldmap->GetY2());

	for (unsigned int x=0; x<old_widthx; x++) {
		for (unsigned int y=0; y<old_widthy; y++) {
			heightmap->SetElementRaw(2*x, 2*y, oldmap->GetElementRaw(x,y));
		}
	}

	for (unsigned int x=0; x<old_widthx-1; x++) {
		for (unsigned int y=0; y<old_widthy-1; y++) {
			float mean = (oldmap->GetElementRaw(x,y)
				+ oldmap->GetElementRaw(x+1,y)
				+ oldmap->GetElementRaw(x,y+1)
				+ oldmap->GetElementRaw(x+1,y+1))/4.0f;
			mean += range*(float(rand())/float(RAND_MAX)-0.5);
			//cout << "*******" << mean << endl;
			heightmap->SetElementRaw(2*x+1, 2*y+1, mean);
		}
	}
	
	for (unsigned int x=0; x<old_widthx; x++) {
		for (unsigned int y=0; y<old_widthy-1; y++) {
			if (x==0) {
				float mean = (heightmap->GetElementRaw(0,2*y)
				+ heightmap->GetElementRaw(0,2*y+2)
				+ heightmap->GetElementRaw(1,2*y+1))/3.0f;
				mean += range*(float(rand())/float(RAND_MAX)-0.5);
				heightmap->SetElementRaw(0, 2*y+1, mean);
			} else if (x==(old_widthx-1)) {
				float mean = (heightmap->GetElementRaw(2*x,2*y)
				+ heightmap->GetElementRaw(2*x,2*y+2)
				+ heightmap->GetElementRaw(2*x-1,2*y+1))/3.0f;
				mean += range*(float(rand())/float(RAND_MAX)-0.5);
				heightmap->SetElementRaw(2*x, 2*y+1, mean);
			} else {
				float mean = (heightmap->GetElementRaw(2*x,2*y)
				+ heightmap->GetElementRaw(2*x,2*y+2)
				+ heightmap->GetElementRaw(2*x-1,2*y+1)
				+ heightmap->GetElementRaw(2*x-1,2*y-1))/4.0f;
				mean += range*(float(rand())/float(RAND_MAX)-0.5);
				heightmap->SetElementRaw(2*x, 2*y+1, mean);
			} 
		}
	}

	for (unsigned int y=0; y<old_widthy; y++) {
		for (unsigned int x=0; x<old_widthx-1; x++) {
			if (y==0) {
				float mean = (heightmap->GetElementRaw(2*x,0)
				+ heightmap->GetElementRaw(2*x+2,0)
				+ heightmap->GetElementRaw(2*x+1,1))/3.0f;
				mean += range*(float(rand())/float(RAND_MAX)-0.5);
				heightmap->SetElementRaw(2*x+1, 0, mean);
			} else if (y==(old_widthy-1)) {
				float mean = (heightmap->GetElementRaw(2*x,2*y)
				+ heightmap->GetElementRaw(2*x+2,2*y)
				+ heightmap->GetElementRaw(2*x+1,2*y-1))/3.0f;
				mean += range*(float(rand())/float(RAND_MAX)-0.5);
				heightmap->SetElementRaw(2*x+1, 2*y, mean);
			} else {
				float mean = (heightmap->GetElementRaw(2*x,2*y)
				+ heightmap->GetElementRaw(2*x+2,2*y)
				+ heightmap->GetElementRaw(2*x+1,2*y-1)
				+ heightmap->GetElementRaw(2*x+1,2*y+1))/4.0f;
				mean += range*(float(rand())/float(RAND_MAX)-0.5);
				heightmap->SetElementRaw(2*x+1, 2*y, mean);
			} 
		}
	}

	delete (oldmap);
	_llMapList()->ExchangeMap(mapname, heightmap);

	return 1;
}

#include "..\include\llfilter.h"
#include "..\include\llmaplist.h"
#include "..\include\llmakederivatives.h"


//constructor
llFilter::llFilter() : llWorker() {

	SetCommandName("Filter");
	sourcename = NULL;
	targetname = NULL;
	makeshort = 0;
	overwrite = 0;
	makederivatives = 0;
}

int llFilter::RegisterOptions(void) {
	if (!llWorker::RegisterOptions()) return 0;

	RegisterValue("-source", &sourcename);
	RegisterValue("-target", &targetname);
	RegisterValue("-n",      &dist);
	RegisterFlag("-use16bit", &makeshort);
	RegisterFlag("-overwrite", &overwrite);
	RegisterFlag("-MakeDerivatives", &makederivatives);

	return 1;
}

int llFilter::Init(void) {
	if (!llWorker::Init()) return 0;

	if (!Used("-source"))
		sourcename = "_heightmap";

	if (!Used("-target")) {
		targetname = new char[strlen(sourcename)+10];
		sprintf_s(targetname, strlen(sourcename)+10, "%s_filtered", sourcename);
	}

	llMap *oldmap = _llMapList()->GetMap(sourcename);

	if (!oldmap) {
		_llLogger()->WriteNextLine(-LOG_ERROR,"%s: map %s not found", command_name, sourcename);
		return 0;
	}

	llMap *newmap = _llMapList()->GetMap(targetname);

	if (newmap) {
		_llLogger()->WriteNextLine(-LOG_WARNING,"%s: map %s existing, going to delete it", command_name, targetname);
		_llMapList()->DeleteMap(targetname);
	}

	int widthx = oldmap->GetWidthX();
	int widthy = oldmap->GetWidthY();

	float defaultheight = oldmap->GetDefaultHeight();
	newmap = new llMap(widthx, widthy, makeshort, defaultheight);
//	float minheight = defaultheight + 1.0f;

	for (int y=0; y<widthy; y++) {
		for (int x=0; x<widthx; x++) {
			int x1 = x - dist;
			int x2 = x + dist;
			int y1 = y - dist;
			int y2 = y + dist;
			if (x1 < 0) x1 = 0;
			if (y1 < 0) y1 = 0;
			if (x2 > (int(widthx)-1)) x2 = widthx-1;
			if (y2 > (int(widthy)-1)) y2 = widthy-1;
			float mean=0., num=0.;

			for (int  xx=x1; xx<=x2; xx++) {
				for (int  yy=y1; yy<=y2; yy++) {
					float height = oldmap->GetElementRaw(xx,yy);
//					if (height > minheight) {
						mean +=  height;
						num++;
//					}
				}
			}
			if (num)
				newmap->SetElementRaw(x,y,mean/num);
			else
				//newmap->SetElementRaw(x,y,defaultheight);
				newmap->SetElementRaw(x,y,-15);
		}
	}

	newmap->SetCoordSystem(oldmap->GetX1(), oldmap->GetY1(), oldmap->GetX2(), oldmap->GetY2(), oldmap->GetZScale());
	
	if (overwrite) {
		for (int y=0; y<widthy; y++) {
			for (int x=0; x<widthx; x++) {
				oldmap->SetElementRaw(x, y, newmap->GetElementRaw(x,y));
			}
		}
		delete newmap;
		newmap = oldmap;
	}
	
	//Filtered map shares the points, etc with its master
	llPointList    * points    = _llMapList()->GetPointList(sourcename);
	llTriangleList * triangles = _llMapList()->GetTriangleList(sourcename);
	llPolygonList  * polygons  = _llMapList()->GetPolygonList(sourcename);

	_llMapList()->AddMap(targetname, newmap, points, triangles, polygons);

	if (makederivatives) {
		llMakeDerivatives *der = new llMakeDerivatives();
		if (!der->RegisterOptions()) return 0;
		//cout << der->SetValue("-source", targetname) << endl;
		if (!der->Init()) return 0;
		_llMapList()->ExchangeMap(targetname, oldmap);
	}

	return 1;
}

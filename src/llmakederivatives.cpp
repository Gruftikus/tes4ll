#include "..\include\llmakederivatives.h"
#include "..\include\llmaplist.h"

//constructor
llMakeDerivatives::llMakeDerivatives() : llWorker() {

	SetCommandName("MakeDerivatives");
	sourcename = NULL;
	makeshort = 0;
}

int llMakeDerivatives::RegisterOptions(void) {
	if (!llWorker::RegisterOptions()) return 0;

	RegisterValue("-source", &sourcename);
	RegisterFlag("-use16bit", &makeshort);

	return 1;
}

int llMakeDerivatives::Init(void) {
	if (!llWorker::Init()) return 0;

	if (!Used("-source"))
		sourcename = "_heightmap";

	llMap *oldmap = _llMapList()->GetMap(sourcename);

	if (!oldmap) {
		_llLogger()->WriteNextLine(-LOG_ERROR,"%s: map %s not found", command_name, sourcename);
		return 0;
	}

	unsigned int widthx = oldmap->GetWidthX();
	unsigned int widthy = oldmap->GetWidthY();

	//Filtered map shares the points, etc with its master
	llPointList    * points    = _llMapList()->GetPointList(sourcename);
	llTriangleList * triangles = _llMapList()->GetTriangleList(sourcename);
	llPolygonList  * polygons  = _llMapList()->GetPolygonList(sourcename);

	char * namex1 = new char[strlen(sourcename)+5];
	sprintf_s(namex1, strlen(sourcename)+5, "%s_d1x", sourcename);
	llMap *mapx1 = _llMapList()->GetMap(namex1);
	if (mapx1) {
		_llLogger()->WriteNextLine(-LOG_WARNING,"%s: derivative map '%s' exists, going to delete it", command_name, namex1);
		_llMapList()->DeleteMap(namex1);
	}
	char * namey1 = new char[strlen(sourcename)+5];
	sprintf_s(namey1, strlen(sourcename)+5, "%s_d1y", sourcename);
	llMap *mapy1 = _llMapList()->GetMap(namey1);
	if (mapy1) {
		_llLogger()->WriteNextLine(-LOG_WARNING,"%s: derivative map '%s' exists, going to delete it", command_name, namey1);
		_llMapList()->DeleteMap(namey1);
	}
	char * namex2 = new char[strlen(sourcename)+5];
	sprintf_s(namex2, strlen(sourcename)+5, "%s_d2x", sourcename);
	llMap *mapx2 = _llMapList()->GetMap(namex2);
	if (mapx2) {
		_llLogger()->WriteNextLine(-LOG_WARNING,"%s: derivative map '%s' exists, going to delete it", command_name, namex2);
		_llMapList()->DeleteMap(namex2);
	}
	char * namey2 = new char[strlen(sourcename)+5];
	sprintf_s(namey2, strlen(sourcename)+5, "%s_d2y", sourcename);
	llMap *mapy2 = _llMapList()->GetMap(namey2);
	if (mapy2) {
		_llLogger()->WriteNextLine(-LOG_WARNING,"%s: derivative map '%s' exists, going to delete it", command_name, namey2);
		_llMapList()->DeleteMap(namey2);
	}

	llShortarray *data1x;
	llShortarray *data1y;
	llShortarray *data2x;
	llShortarray *data2y;

	///float minheight = oldmap->GetDefaultHeight() + 1.0f;
	float minheight = -9999;
	int redone = 0;

repeat:

	data1x = new llShortarray(widthx*widthy, makeshort); 
	data1y = new llShortarray(widthx*widthy, makeshort); 	
	data2x = new llShortarray(widthx*widthy, makeshort); 	
	data2y = new llShortarray(widthx*widthy, makeshort); 

	if (!data1x->SetElement(0,0.f) || !data1y->SetElement(0,0.f) || 
		!data2x->SetElement(0,0.f) || !data2y->SetElement(0,0.f)) { //test bad alloc
		if (!makeshort) {
			makeshort = 1;
			redone    = 1;
			_llLogger()->WriteNextLine(LOG_WARNING, "%s: memory allocation failed, I will try the 16bit version", command_name);
			delete data1x;
			delete data1y;
			delete data2x;
			delete data2y;
			data1x = data1y = data2x = data2y = NULL;
			goto repeat;
		} else {
			_llLogger()->WriteNextLine(-LOG_FATAL,"%s: out of memory", command_name);
		}
	}

	if (redone) {
		_llLogger()->AddToLine("... [OK]");
		_llLogger()->Dump();
	}

	float widthx_per_raw = oldmap->GetWidthXPerRaw();
	float widthy_per_raw = oldmap->GetWidthYPerRaw();

	x1max=0;
	for (unsigned int y=0; y<widthy; y++) {
		data1x->SetElement(y*widthx, (oldmap->GetElementRaw(1,y) - oldmap->GetElementRaw(0,y)));
		data1x->SetElement(y*widthx+widthx-1, (oldmap->GetElementRaw(widthx-1,y) - oldmap->GetElementRaw(widthx-2,y)));
		for (unsigned int x=1; x<widthx-1; x++) {
			if ((oldmap->GetElementRaw(x-1,y) > minheight && oldmap->GetElementRaw(x+1,y) > minheight)) {
				data1x->SetElement(x+y*widthx, (oldmap->GetElementRaw(x-1,y) - oldmap->GetElementRaw(x+1,y)) / (2.0f));
				//cout << (oldmap->GetElementRaw(x-1,y) - oldmap->GetElementRaw(x+1,y)) / (2.0f) << endl;
				if (fabs(((*data1x)[x+y*widthx])) > x1max) 
					x1max = fabs(((*data1x)[x+y*widthx]));
			}
		}
	}

	y1max=0;
	for (unsigned int x=0; x<widthx; x++) {
		data1y->SetElement(x, (oldmap->GetElementRaw(x,1) - oldmap->GetElementRaw(x,0)));
		data1y->SetElement(widthx+(widthy-1)*widthx, (oldmap->GetElementRaw(x,widthy-1) - oldmap->GetElementRaw(x,widthy-2)));
		for (unsigned int y=1; y<widthy-1; y++) {
			if ((oldmap->GetElementRaw(x,y-1) > minheight && oldmap->GetElementRaw(x,y+1) > minheight)) {
				data1y->SetElement(x+y*widthx, (oldmap->GetElementRaw(x,y-1) - oldmap->GetElementRaw(x,y+1)) / (2.0f));
				if (fabs(((*data1y)[x+y*widthx])) > y1max) 
					y1max = fabs(((*data1y)[x+y*widthx]));
			}
		}
	}

	mapx1 = new llMap(widthx, widthy, data1x , 0);
	mapx1->SetCoordSystem(oldmap->GetX1(), oldmap->GetY1(), oldmap->GetX2(), oldmap->GetY2(), oldmap->GetZScale()/widthx_per_raw);
	_llMapList()->AddMap(namex1, mapx1, points, triangles, polygons);
	mapy1 = new llMap(widthx, widthy, data1y , 0);
	mapy1->SetCoordSystem(oldmap->GetX1(), oldmap->GetY1(), oldmap->GetX2(), oldmap->GetY2(), oldmap->GetZScale()/widthy_per_raw);
	_llMapList()->AddMap(namey1, mapy1, points, triangles, polygons);

	x2max=0;
	for (unsigned int y=0; y<widthy; y++) {
		data1x->SetElement(y*widthx, mapx1->GetElementRaw(1,y) - mapx1->GetElementRaw(0,y));
		data1x->SetElement(y*widthx+widthx-1, mapx1->GetElementRaw(widthx-1,y) - mapx1->GetElementRaw(widthx-2,y));
		for (unsigned int x=1; x<widthx-1; x++) {
			if ((oldmap->GetElementRaw(x-1,y) > minheight && oldmap->GetElementRaw(x+1,y) > minheight)) {
				data2x->SetElement(x+y*widthx, (mapx1->GetElementRaw(x-1,y) - mapx1->GetElementRaw(x+1,y)) / 2.0f);
				if (fabs(((*data2x)[x+y*widthx])) > x2max) 
					x1max = fabs(((*data2x)[x+y*widthx]));
			}
		}
	}

	y2max=0;
	for (unsigned int x=0; x<widthx; x++) {
		data1y->SetElement(x, mapy1->GetElementRaw(x,1) - mapy1->GetElementRaw(x,0));
		data1y->SetElement(widthx+(widthy-1)*widthx, mapy1->GetElementRaw(x,widthy-1) - mapy1->GetElementRaw(x,widthy-2));
		for (unsigned int y=1; y<widthy-1; y++) {
			if ((oldmap->GetElementRaw(x,y-1) > minheight && oldmap->GetElementRaw(x,y+1) > minheight)) {
				data2y->SetElement(x+y*widthx, (mapy1->GetElementRaw(x,y-1) - mapy1->GetElementRaw(x,y+1)) / 2.0f);
				if (fabs(((*data2y)[x+y*widthx])) > y2max) 
					y1max = fabs(((*data2y)[x+y*widthx]));
			}
		}
	}

	data1x->Print(namex1, _llLogger());
	data1y->Print(namey1, _llLogger());
	data2x->Print(namex2, _llLogger());
	data2y->Print(namey2, _llLogger());

	mapx2 = new llMap(widthx, widthy, data2x, 0);
	mapx2->SetCoordSystem(oldmap->GetX1(), oldmap->GetY1(), oldmap->GetX2(), oldmap->GetY2(), oldmap->GetZScale()/widthx_per_raw);
	_llMapList()->AddMap(namex2, mapx2, points, triangles, polygons);
	mapy2 = new llMap(widthx, widthy, data2y, 0);
	mapy2->SetCoordSystem(oldmap->GetX1(), oldmap->GetY1(), oldmap->GetX2(), oldmap->GetY2(), oldmap->GetZScale()/widthy_per_raw);
	_llMapList()->AddMap(namey2, mapy2, points, triangles, polygons);

	return 1;
}

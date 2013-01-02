#include "..\include\llexportmap.h"
#include "..\include\llmaplist.h"

//constructor
llExportMap::llExportMap() : llWorker() {

	SetCommandName("ExportMap");
	mapname  = NULL;
	filename = NULL;

}

int llExportMap::RegisterOptions(void) {
	if (!llWorker::RegisterOptions()) return 0;

	RegisterValue("-mapname",  &mapname);
	RegisterValue("-filename", &filename);
	RegisterValue("-depth",    &bits);

	return 1;
}

int llExportMap::Init(void) {
	if (!llWorker::Init()) return 0;

	if (!Used("-mapname"))
		mapname = "_heightmap";
	if (!Used("-filename"))
		filename = "world.bmp";
	if (!Used("-depth"))
		bits = 24;

	llMap * map = _llMapList()->GetMap(mapname);

	FILE *fptr;

	if (fopen_s(&fptr, filename,"wb")) {
		_llLogger()->WriteNextLine(-LOG_ERROR,"Unable to open BMP file '%s'\n", filename);
		return 0;
	}

	int x1 = map->GetRawX(_llUtils()->x00);
	int y1 = map->GetRawY(_llUtils()->y00);
	int x2 = map->GetRawX(_llUtils()->x11);
	int y2 = map->GetRawY(_llUtils()->y11);

	std::cout << x1 << ":" << y1 << ":" << x2 << ":" << y2 << endl;

	INFOHEADER infoheader;
	infoheader.width = x2-x1+1;
	infoheader.height= y2-y1+1;
	infoheader.compression=0;
	infoheader.size=40;  
	infoheader.planes=1;       /* Number of colour planes   */
	if (bits == 24)
		infoheader.bits=24;         /* Bits per pixel            */
	else
		infoheader.bits=32;
	if (bits == 24)
		infoheader.imagesize=3*infoheader.width*infoheader.height;          /* Image size in bytes       */
	else
		infoheader.imagesize=4*infoheader.width*infoheader.height;
	infoheader.xresolution=100;
	infoheader.yresolution=100;     /* Pixels per meter          */
	infoheader.ncolours=0;           /* Number of colours         */
	infoheader.importantcolours;   /* Important colours         */

	HEADER header;
	header.type='M'*256+'B';
	header.size=14+40+infoheader.imagesize;
	header.reserved1=header.reserved2=0;
	WriteUShort(fptr,header.type,0);
	WriteUInt(fptr,header.size,0);
	WriteUShort(fptr,header.reserved1,0);
	WriteUShort(fptr,header.reserved2,0);
	header.offset=14+40;
	WriteUInt(fptr,header.offset,0);

	/* Read and check the information header */
	if (fwrite(&infoheader,sizeof(INFOHEADER),1,fptr) != 1) {
		_llLogger()->WriteNextLine(-LOG_ERROR,"Failed to write BMP info header");
		return 0;
	}

	for (int y=y2; y>=y1; y--) {
		for (int x=x2; x>=x1; x--) {
			float height = float(map->GetZ((unsigned int)x, (unsigned int)y));
			float val = map->GetElementRaw(x,y);
			WriteUInt(fptr,unsigned int(val),0);
		}
	}
	fclose(fptr);

	return 1;
}

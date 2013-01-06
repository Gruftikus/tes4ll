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
	if (!map) {
		_llLogger()->WriteNextLine(-LOG_ERROR,"%s: map '%s' not found", command_name, mapname);
		return 0;
	}

	FILE *fptr;

	if (fopen_s(&fptr, filename,"wb")) {
		_llLogger()->WriteNextLine(-LOG_ERROR,"Unable to open BMP file '%s'\n", filename);
		return 0;
	}

	int x1 = map->GetRawX(_llUtils()->x00);
	int y1 = map->GetRawY(_llUtils()->y00);
	int x2 = map->GetRawX(_llUtils()->x11);
	int y2 = map->GetRawY(_llUtils()->y11);

	INFOHEADER infoheader;
	infoheader.width  = x2-x1+1;
	infoheader.height = y2-y1+1;
	infoheader.compression=0;
	infoheader.size   = 40;  
	infoheader.planes = 1;       /* Number of colour planes   */
	int bytesPerLine;
	if (bits == 24)
		infoheader.bits = 24;         /* Bits per pixel            */
	else
		infoheader.bits = 32;
	if (bits == 24) {
		bytesPerLine = infoheader.width * 3;  /* (for 24 bit images) */
		/* round up to a dword boundary */
		//Thanks to 
		//http://www.siggraph.org/education/materials/HyperVis/asp_data/compimag/bmpfile.htm
		//for the hint
		if (bytesPerLine & 0x0003) {
			bytesPerLine |= 0x0003;
			++bytesPerLine;
		}		     
	} else
		bytesPerLine = infoheader.width * 4;
	infoheader.imagesize = (long)bytesPerLine*infoheader.height;          /* Image size in bytes  */
	infoheader.xresolution=100;
	infoheader.yresolution=100;     /* Pixels per meter          */
	infoheader.ncolours=0;          /* Number of colours         */
	infoheader.importantcolours;    /* Important colours         */

	HEADER header;
	header.type='M'*256+'B';
	header.size = 14 + 40 + infoheader.imagesize;
	header.reserved1 = header.reserved2 = 0;
	WriteUShort(fptr, header.type, 0);
	WriteUInt  (fptr, header.size, 0);
	WriteUShort(fptr, header.reserved1, 0);
	WriteUShort(fptr, header.reserved2, 0);
	header.offset = 14+40;
	WriteUInt(fptr, header.offset, 0);

	/* Read and check the information header */
	if (fwrite(&infoheader, sizeof(INFOHEADER), 1, fptr) != 1) {
		_llLogger()->WriteNextLine(-LOG_ERROR,"Failed to write BMP info header");
		return 0;
	}

	for (int y=y2; y>=y1; y--) {
		int my_bytesPerLine = bytesPerLine;
		for (int x=x2; x>=x1; x--) {
			
			char byte1;
			char byte2;
			char byte3;
			char byte4;

			if (map->GetTupel(x, y, &byte1, &byte2, &byte3, &byte4)) {
				if (bits == 24) {
					my_bytesPerLine -= 3;
					fwrite(&byte1, 1, 1, fptr); //blue
					fwrite(&byte2, 1, 1, fptr); //green
					fwrite(&byte3, 1, 1, fptr); //red
				} else {
					fwrite(&byte1, 1, 1, fptr); //blue
					fwrite(&byte2, 1, 1, fptr); //green
					fwrite(&byte3, 1, 1, fptr); //red
					fwrite(&byte4, 1, 1, fptr); //alpha
					my_bytesPerLine -= 4;
				}
			}

#if 0
			float val = map->GetElementRaw(x,y);
			if (bits == 24) {
				my_bytesPerLine -= 3;
				unsigned int trunk1 = int(val) & 0xff;
				unsigned int trunk2 = (int(val) & 0xff00) >> 8;
				unsigned int trunk3 = (int(val) & 0xff0000) >> 16;
				fwrite(&trunk1, 1, 1, fptr); //blue
				fwrite(&trunk2, 1, 1, fptr); //green
				fwrite(&trunk3, 1, 1, fptr); //red
			} else {
				WriteUInt(fptr,unsigned int(val), 0);
				my_bytesPerLine -= 4;
			}
#endif
		}
		while(my_bytesPerLine) {
			my_bytesPerLine--;
			unsigned int trunk1 = 0xff;
			fwrite(&trunk1, 1, 1, fptr);
		}
	}
	fclose(fptr);

	return 1;
}

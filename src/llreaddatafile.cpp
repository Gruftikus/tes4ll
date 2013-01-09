#include "..\include\llreaddatafile.h"
#include <string.h>
#include <stdio.h>

//constructor
llReadDataFile::llReadDataFile() : llSet() {

	SetCommandName("ReadDataFile");
}

int llReadDataFile::RegisterOptions(void) {
	if (!llSet::RegisterOptions()) return 0;

	RegisterValue("-filename", &filename, LLWORKER_OBL_OPTION);

	return 1;
}


int llReadDataFile::Init(void) {
	if (!llSet::Init()) return 0;

	FILE *fptr_data;

	if (fopen_s(&fptr_data, filename, "r")) {
		_llLogger()->WriteNextLine(-LOG_FATAL,"Unable to open Data file \"%s\"", filename);
	}

	char line[1000];
	char *linex = line;
	size_t size = 1000;
	int lp = 0;
	while (fgets(line, 1000, fptr_data)) {
		_llUtils()->StripSpaces(&linex);
		float x, y;
		if ((strlen(linex)>2) && (linex[0]!=';') && (linex[0]!='#') && (linex[0]!='[')) {
			if (sscanf_s(line,"%f %f",&x,&y) == 2) {
				if (!heightmap->IsInMap(x,y)) {
					_llLogger()->WriteNextLine(LOG_ERROR, "Point (%.0f,%.0f) not in map", x, y);
				} else {
					points->AddPoint(x, y, heightmap->GetZ(x,y));	
					lp++;
				}
			} else 
				_llLogger()->WriteNextLine(LOG_ERROR,"Syntax error in data file \"%s\" in the line: %i", filename, line);
		}
	}
	_llLogger()->WriteNextLine(LOG_INFO,"%s: %i vertex points added from data file %s", command_name, lp, filename);

	return 1;
}
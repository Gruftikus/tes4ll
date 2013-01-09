#include "..\include\llreadpolygondatafile.h"
#include <string.h>
#include <stdio.h>

//constructor
llReadPolygonDataFile::llReadPolygonDataFile() : llSet() {

	SetCommandName("ReadPolygonDataFile");

}

int llReadPolygonDataFile::RegisterOptions(void) {
	if (!llSet::RegisterOptions()) return 0;

	RegisterValue("-filename", &filename);
	RegisterValue("-name",     &polygon_name, LLWORKER_OBL_OPTION);

	return 1;
}


int llReadPolygonDataFile::Init(void) {
	if (!llSet::Init()) return 0;

	llPolygonList *polygons = _llMapList()->GetPolygonList(map);
	if (!polygons) {
		_llLogger()->WriteNextLine(-LOG_FATAL, "%s: no polygon list in map [%s]", command_name, map);
		return 0;
	}

	FILE * file;
	if (fopen_s(&file, filename, "r")) {
		_llLogger()->WriteNextLine(-LOG_ERROR,"Unable to open %s", filename);
		return 0;
	} else {
		char line[1000];
		char *linenew;
		char *current_polygon = NULL;
		int num_vertex = 0;
		float x1, y1, x2, y2;
		while (fgets(line,1000,file)) {
			linenew = line;
			_llUtils()->StripSpaces(&linenew);
			if (strlen(linenew)) {
				if (linenew[0] == '[') {
					//new polygon name
					for (unsigned int i=1;i<strlen(linenew);i++) 
						if (linenew[i] == ']') linenew[i] ='\0';
					if (current_polygon) 
						delete [] current_polygon;
					current_polygon = new char[strlen(linenew+1)+1];
					strcpy_s(current_polygon, strlen(linenew+1)+1, linenew+1);
					num_vertex = 0;
				} else if (linenew[0] != '#' && linenew[0] != ';') {
					//read coordinates
					if (!current_polygon) {
						_llLogger()->WriteNextLine(LOG_ERROR,"%s: no polygon section for [%s]", command_name, linenew);
					} else {
						if (!polygon_name || _stricmp(polygon_name, current_polygon)==0) {
							num_vertex++;
							if (num_vertex == 1) {
								sscanf_s(linenew, "%f %f", &x1, &y1);
							} else if (num_vertex == 2) {
								sscanf_s(linenew, "%f %f", &x2, &y2);
								polygons->AddPolygon(x1, y1, x2, y2, current_polygon);    	    		
							} else {
								sscanf_s(linenew, "%f %f", &x1, &y1);
								polygons->AddVertexToPolygon(x1, y1, current_polygon);    
							}
						}
					}
				}
			}
		} //while
		if (current_polygon) delete [] current_polygon;
	} //fopen

}
#include "../include/llreadlodsettings.h"
#include "../../lltool/include/llutils.h"



llReadLodSettings::llReadLodSettings() : llWorker() {
	SetCommandName("ReadLodSettings");
}

int llReadLodSettings::Prepare(void) {
	if (!llWorker::Prepare()) return 0;

	filename = NULL;

	return 1;
}

int llReadLodSettings::RegisterOptions(void) {
	if (!llWorker::RegisterOptions()) return 0;

	RegisterValue("-filename", &filename);

	return 1;
}

int llReadLodSettings::Exec(void) {
	if (!llWorker::Exec()) return 0;

	if (!Used("-filename") && !_llUtils()->size) {
		_llLogger()->WriteNextLine(-LOG_ERROR, "%s: no filename given, but no data in memory", command_name);
		return 0;
	}

	FILE *fptr = NULL;

	if (filename) {
		if (fopen_s(&fptr, filename," rb")) {
			_llLogger()->WriteNextLine(-LOG_ERROR, "Unable to open LODSettings file \"%s\"", filename);
			return 0;
		}

		ReadInt(fptr, &unknown1, 0);
		ReadInt(fptr, &unknown2, 0);
		ReadInt(fptr, &unknown3, 0);
		ReadShort(fptr, &x1, 0);
		ReadShort(fptr, &y1, 0);
		ReadShort(fptr, &x2, 0);
		ReadShort(fptr, &y2, 0);
		ReadInt(fptr, &unknown4, 0);

	} else {

		if (_llUtils()->size < 24) {
			_llLogger()->WriteNextLine(-LOG_ERROR, "%s: data size (%i) too small", command_name, _llUtils()->size);
			return 0;
		}

		x1 = *(signed short int*)(_llUtils()->data + 12);
		y1 = *(signed short int*)(_llUtils()->data + 14);
		x2 = *(signed short int*)(_llUtils()->data + 16);
		y2 = *(signed short int*)(_llUtils()->data + 18);


	}

	_llUtils()->SetValue("_lod_x1", x1);
	_llUtils()->SetValue("_lod_y1", y1);
	_llUtils()->SetValue("_lod_x2", x2);
	_llUtils()->SetValue("_lod_y2", y2);

	if (filename) 
		_llLogger()->WriteNextLine(-LOG_INFO, "%s: read dimensions x1: %i, y1: %i, x2: %i, y2: %i from file %s", command_name, x1, y1, x2, y2, filename);
	else           
		_llLogger()->WriteNextLine(-LOG_INFO, "%s: read dimensions x1: %i, y1: %i, x2: %i, y2: %i from memory", command_name, x1, y1, x2, y2);

	return 1;
}

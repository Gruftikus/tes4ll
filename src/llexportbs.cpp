#include "../include/llexportbs.h"

#include "../../lltool/include/llmaplist.h"

//#include "../../niflib/include/obj/NiTexturingProperty.h"

//constructor
llExportBS::llExportBS() : llExportMeshToNif() {
	SetCommandName("ExportBS");
}

int llExportBS::Prepare(void) {
	if (!llExportMeshToNif::Prepare()) return 0;

	writeonly = 0;

	return 1;
}

int llExportBS::RegisterOptions(void) {
	if (!llExportMeshToNif::RegisterOptions()) return 0;

	RegisterFlag ("-writeonly",  &writeonly);

	return 1;
}


int llExportBS::Exec(void) {

	char *myfile = filename;

	//look for _install_dir:
	if (_llUtils()->GetValue("_install_dir") && filename) {
		char *filename_tmp = new char[strlen(filename) + strlen(_llUtils()->GetValue("_install_dir")) + 2];
		sprintf_s(filename_tmp, strlen(filename) + strlen(_llUtils()->GetValue("_install_dir")) + 2, "%s\\%s", 
			_llUtils()->GetValue("_install_dir"), filename);
		myfile = filename_tmp;
	}

	//Now the nif-specific part:

	using namespace Niflib;

	if (!ninode_ptr) {
		ninode_ptr = new BSMultiBoundNode;
	}

	if (!writeonly) {
		//exporting base mesh:
		llExportMeshToNif::Exec();
	}

	NifInfo info = NifInfo();
	info.version = 335675399;
	info.userVersion = 11;
	info.userVersion2 = 34;
	if (_llUtils()->GetValue("_nif_version"))
		sscanf(_llUtils()->GetValue("_nif_version"), "%u", &(info.version));

	if (myfile) {
		//std::cout << myfile << std::endl;
		WriteNifTree(myfile, ninode_ptr, info);
		ninode_ptr = NULL;
	}

	if (_llUtils()->GetValue("_install_dir") && myfile) {
		delete myfile;
	}

	return 1;
}

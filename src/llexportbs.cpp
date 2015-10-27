#include "../include/llexportbs.h"

#include "../../lltool/include/llmaplist.h"

#include "../../niflib/include/obj/BSMultiBoundAABB.h"
#include "../../niflib/include/obj/BSMultiBound.h"

//constructor
llExportBS::llExportBS() : llExportMeshToNif() {
	SetCommandName("ExportBS");
}

int llExportBS::Prepare(void) {
	if (!llExportMeshToNif::Prepare()) return 0;

	writeonly = makebound = 0;

	return 1;
}

int llExportBS::RegisterOptions(void) {
	if (!llExportMeshToNif::RegisterOptions()) return 0;

	RegisterFlag ("-writeonly",  &writeonly);
	RegisterFlag ("-makebound",  &makebound);

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
		ninode_ptr->SetFlags(2062);
	}

	if (!writeonly) {
		//exporting base mesh:
		llExportMeshToNif::Exec();
	}

	if (makebound) {
		BSMultiBoundRef     mbnd = new BSMultiBound;
		BSMultiBoundAABBRef aabb = new BSMultiBoundAABB;

		aabb->SetAABB(Vector3((xmax + xmin)*0.5f, (ymax + ymin)*0.5f, (zmax + zmin)*0.5f), 
			Vector3((xmax - xmin)*0.5f, (ymax - ymin)*0.5f, (zmax - zmin)*0.5f));
		mbnd->SetData(aabb);

		((BSMultiBoundNode*)ninode_ptr)->SetMultiBound(mbnd);
	}

	if (myfile) {
		NifInfo info = NifInfo();
		info.version = 335675399;
		info.userVersion = 11;
		info.userVersion2 = 34;
		if (_llUtils()->GetValue("_nif_version"))
			sscanf(_llUtils()->GetValue("_nif_version"), "%u", &(info.version));
		ninode_ptr->SetLocalTranslation(Vector3 (loc_trans_x, loc_trans_y, loc_trans_z));
		WriteNifTree(myfile, ninode_ptr, info);
		ninode_ptr = NULL;
	}

	if (_llUtils()->GetValue("_install_dir") && myfile) {
		delete myfile;
	}

	return 1;
}

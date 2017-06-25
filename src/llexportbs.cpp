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
	loc_bsnode = 0;
	flags = 2062;

	return 1;
}

int llExportBS::RegisterOptions(void) {
	if (!llExportMeshToNif::RegisterOptions()) return 0;

	RegisterFlag ("-writeonly",  &writeonly);
	RegisterFlag ("-makebound",  &makebound);
	RegisterFlag ("-makelocalbsnode", &loc_bsnode);

	RegisterValue("-flags", &flags);

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
		ninode_ptr->SetFlags(flags);
	}

	Niflib::NiNode *tmp=NULL;
	if (loc_bsnode) {
		tmp = ninode_ptr;
		ninode_ptr = new BSMultiBoundNode;
		ninode_ptr->SetFlags(flags);
	}

	if (setname)
		ninode_ptr->SetName(setname);

	if (!writeonly) {
		if (setname && loc_bsnode) setname=NULL;
		//exporting base mesh:
		llExportMeshToNif::Exec();
	}

	if (makebound) {
		BSMultiBoundRef     mbnd = new BSMultiBound;
		BSMultiBoundAABBRef aabb = new BSMultiBoundAABB;

		xmin += trans_x/scale;
		xmax += trans_x/scale;
		ymin += trans_y/scale;
		ymax += trans_y/scale;
		zmin += trans_z/scale;
		zmax += trans_z/scale;

		aabb->SetAABB(Vector3((xmax + xmin)*0.5f, (ymax + ymin)*0.5f, (zmax + zmin)*0.5f), 
			Vector3((xmax - xmin)*0.5f, (ymax - ymin)*0.5f, (zmax - zmin)*0.5f));
		mbnd->SetData(aabb);

		((BSMultiBoundNode*)ninode_ptr)->SetMultiBound(mbnd);
	}

	if (tmp) {
		tmp->AddChild(ninode_ptr);
		ninode_ptr = tmp;
	}

	if (myfile) {
		NifInfo info = NifInfo();
		info.version = 335675399;
		info.userVersion = 11;
		//info.userVersion = 12;
		info.userVersion2 = 34;
		//info.userVersion2 = 83;
		if (_llUtils()->GetValue("_nif_version"))
			sscanf(_llUtils()->GetValue("_nif_version"), "%u", &(info.version));
		if (_llUtils()->GetValue("_nif_userversion1"))
			sscanf(_llUtils()->GetValue("_nif_userversion1"), "%u", &(info.userVersion));
		if (_llUtils()->GetValue("_nif_userversion2"))
			sscanf(_llUtils()->GetValue("_nif_userversion2"), "%u", &(info.userVersion2));
		ninode_ptr->SetLocalTranslation(Vector3 (loc_trans_x, loc_trans_y, loc_trans_z));
		WriteNifTree(myfile, ninode_ptr, info);
		ninode_ptr = NULL;
	}

	if (_llUtils()->GetValue("_install_dir") && myfile) {
		delete myfile;
	}

	return 1;
}

#include "../include/llexportmeshtonif.h"

#include "../../lltool/include/llmaplist.h"

#include "../../niflib/include/obj/NiNode.h"
#include "../../niflib/include/obj/NiTriStrips.h"
#include "../../niflib/include/obj/NiTriStripsData.h"
#include "../../niflib/include/obj/NiTriShape.h"
#include "../../niflib/include/obj/NiTriShapeData.h"
#include "../../niflib/include/niflib.h"
#include "../../niflib/include/obj/NiTexturingProperty.h"
#include "../../niflib/include/obj/NiSourceTexture.h"

//constructor
llExportMeshToNif::llExportMeshToNif() : llExportMeshToObj() {
	SetCommandName("ExportMeshToNif");

}

int llExportMeshToNif::Prepare(void) {
	if (!llExportMeshToObj::Prepare()) return 0;

	useshapes = 0;

	return 1;
}

int llExportMeshToNif::RegisterOptions(void) {
	if (!llExportMeshToObj::RegisterOptions()) return 0;

	RegisterFlag ("-useshapes", &useshapes);

	return 1;
}


int llExportMeshToNif::Exec(void) {
	if (!llExportMeshToObj::Exec()) return 0;

	if (!Used("-filename"))
	    filename = (char *)"map.nif";
	if (!MakeSelection()) return 0;

	//look for _install_dir:
	if (_llUtils()->GetValue("_install_dir")) {
		char *filename_tmp = new char[strlen(filename) + strlen(_llUtils()->GetValue("_install_dir")) + 2];
		sprintf_s(filename_tmp, strlen(filename) + strlen(_llUtils()->GetValue("_install_dir")) + 2, "%s\\%s", 
			_llUtils()->GetValue("_install_dir"), filename);
		filename = filename_tmp;
	}

	//Now the nif-specific part:

	using namespace Niflib;

	int num_triangles = newtriangles->GetN();
    std::vector<Triangle> t(num_triangles);
	newpoints->Resize();
	newpoints->Translation(trans_x, trans_y, trans_z);

	if (useshapes) {

		NiTriShape* node_ptr = new NiTriShape;
		NiTriShapeRef node = node_ptr;

		NiTriShapeData * node2_ptr = new NiTriShapeData();
		NiTriShapeDataRef node2 = node2_ptr;
		node_ptr->SetData(node2);
		node_ptr->SetFlags(14);

		node2_ptr->SetVertices(reinterpret_cast<std::vector<Niflib::Vector3> & >(newpoints->GetVertices()));   
		node2_ptr->SetTspaceFlag(16);

		for (int i=0;i<num_triangles;i++) {
			t[i].v1 = newtriangles->GetPoint1(i);
			t[i].v2 = newtriangles->GetPoint2(i);
			t[i].v3 = newtriangles->GetPoint3(i);
		}

		node2_ptr->SetTriangles(t);
		node2_ptr->SetUVSetCount(1);
		node2_ptr->SetUVSet(0, reinterpret_cast<std::vector<Niflib::TexCoord> & >(newpoints->GetUV()));

		if (texname) {
			//optional textures
			NiTexturingProperty * texture_ptr = new NiTexturingProperty();
			NiTexturingPropertyRef texture = texture_ptr;
			NiSourceTexture * image_ptr = new NiSourceTexture();
			NiSourceTextureRef image = image_ptr;
			image->SetExternalTexture(texname);
			TexDesc tex;
			tex.source = image_ptr;
			texture_ptr->SetTexture(0,tex);
			node_ptr->AddProperty(texture);
		}

		//int stripcount = node2_ptr->GetStripCount();

		vector<Triangle> newt=node2_ptr->GetTriangles();

		_llLogger()->WriteNextLine(LOG_INFO, "The (shape-based) mesh %s has %i triangles and %i vertices",
			filename, newt.size(), newpoints->GetVertices().size());

		NifInfo info = NifInfo();
		info.version = 335544325;

		WriteNifTree(filename, node, info);

	} else {

		NiTriStrips* node_ptr = new NiTriStrips;
		NiTriStripsRef node = node_ptr;

		NiTriStripsData * node2_ptr = new NiTriStripsData();
		NiTriStripsDataRef node2 = node2_ptr;
		node_ptr->SetData(node2);
		node_ptr->SetFlags(14);

		node2_ptr->SetVertices(reinterpret_cast<std::vector<Niflib::Vector3> & >(newpoints->GetVertices()));   
		node2_ptr->SetTspaceFlag(16);

		newtriangles->Stripification();
		node2_ptr->SetStripCount(1);
		node2_ptr->SetStrip(0,newtriangles->GetVertices());

		node2_ptr->SetUVSetCount(1);
		node2_ptr->SetUVSet(0, reinterpret_cast<std::vector<Niflib::TexCoord> & >(newpoints->GetUV()));

		if (texname) {
			//optional textures
			NiTexturingProperty * texture_ptr = new NiTexturingProperty();
			NiTexturingPropertyRef texture = texture_ptr;
			NiSourceTexture * image_ptr = new NiSourceTexture();
			NiSourceTextureRef image = image_ptr;
			image->SetExternalTexture(texname);
			TexDesc tex;
			tex.source = image_ptr;
			texture_ptr->SetTexture(0,tex);
			node_ptr->AddProperty(texture);
		}

		int stripcount = node2_ptr->GetStripCount();

		vector<Triangle> newt=node2_ptr->GetTriangles();

		_llLogger()->WriteNextLine(LOG_INFO, "The (shape-based) mesh %s has %i triangles and %i vertices",
			filename, newt.size(), newpoints->GetVertices().size());

		NifInfo info = NifInfo();
		info.version = 335544325;

		WriteNifTree(filename, node, info);
	}

	if (_llUtils()->GetValue("_install_dir")) {
		delete filename;
	}

	return 1;
}

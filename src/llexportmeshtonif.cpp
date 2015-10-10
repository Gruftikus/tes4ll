#include "../include/llexportmeshtonif.h"

#include "../../lltool/include/llmaplist.h"

#include "../../niflib/include/obj/NiTexturingProperty.h"
#include "../../niflib/include/obj/NiSourceTexture.h"
#include "../../niflib/include/obj/bsshadertextureset.h"
#include "../../niflib/include/obj/BSShaderPPLightingProperty.h"

Niflib::NiNode *llExportMeshToNif::ninode_ptr = NULL;

//constructor
llExportMeshToNif::llExportMeshToNif() : llExportMeshToObj() {
	SetCommandName("ExportMeshToNif");
}

int llExportMeshToNif::Prepare(void) {
	if (!llExportMeshToObj::Prepare()) return 0;

	useshapes  = 0;
	makeninode = 0;

	texset1 = NULL;
	texset2 = NULL;

	return 1;
}

int llExportMeshToNif::RegisterOptions(void) {
	if (!llExportMeshToObj::RegisterOptions()) return 0;

	RegisterFlag ("-useshapes",  &useshapes);
	RegisterFlag ("-makeninode", &makeninode);

	RegisterValue("-texset1", &texset1);
	RegisterValue("-texset2", &texset2);

	return 1;
}


int llExportMeshToNif::Exec(void) {
	if (!llTriMod::Exec()) return 0;

	if (!Used("-filename"))
	    //filename = (char *)"map.nif";
		filename = NULL;
	if (!MakeSelection()) return 0;

	//look for _install_dir:
	if (_llUtils()->GetValue("_install_dir") && filename) {
		char *filename_tmp = new char[strlen(filename) + strlen(_llUtils()->GetValue("_install_dir")) + 2];
		sprintf_s(filename_tmp, strlen(filename) + strlen(_llUtils()->GetValue("_install_dir")) + 2, "%s\\%s", 
			_llUtils()->GetValue("_install_dir"), filename);
		//std::cout << _llUtils()->GetValue("_install_dir") << ":" << filename << std::endl;
		filename = filename_tmp;
	}

	//Now the nif-specific part:

	using namespace Niflib;

	if (makeninode) {
		ninode_ptr = new NiNode;
	}

	int num_triangles = newtriangles->GetN();
    std::vector<Triangle> t(num_triangles);
	newpoints->Resize();
	newpoints->Translation(trans_x, trans_y, trans_z);

	NiAVObjectRef myavobj;

	if (useshapes) {

		NiTriShape    *node_ptr = new NiTriShape;
		NiTriShapeRef  node     = node_ptr;
		myavobj                 = node_ptr;

		NiTriShapeData   *node2_ptr = new NiTriShapeData();
		NiTriShapeDataRef node2     = node2_ptr;
		
		node_ptr->SetData(node2);
		node_ptr->SetFlags(14);

		node2_ptr->SetVertices(reinterpret_cast<std::vector<Niflib::Vector3> & >(newpoints->GetVertices()));   
		node2_ptr->SetTspaceFlag(16);

		for (int i=0; i<num_triangles; i++) {
			t[i].v1 = newtriangles->GetPoint1(i);
			t[i].v2 = newtriangles->GetPoint2(i);
			t[i].v3 = newtriangles->GetPoint3(i);
		}

		node2_ptr->SetTriangles(t);
		node2_ptr->SetUVSetCount(1);
		node2_ptr->SetUVSet(0, reinterpret_cast<std::vector<Niflib::TexCoord> & >(newpoints->GetUV()));

		//int stripcount = node2_ptr->GetStripCount();

		vector<Triangle> newt=node2_ptr->GetTriangles();

		_llLogger()->WriteNextLine(-LOG_INFO, "The (shape-based) mesh %s has %i triangles and %i vertices",
			filename, newt.size(), newpoints->GetVertices().size());

		NifInfo info = NifInfo();
		info.version = 335544325;

		if (ninode_ptr) 
			ninode_ptr->AddChild(node_ptr);

	} else {

		NiTriStrips   *node_ptr = new NiTriStrips;
		NiTriStripsRef node     = node_ptr;
		myavobj                 = node_ptr;

		NiTriStripsData   *node2_ptr = new NiTriStripsData();
		NiTriStripsDataRef node2     = node2_ptr;

		node_ptr->SetData(node2);
		node_ptr->SetFlags(14);

		node2_ptr->SetVertices(reinterpret_cast<std::vector<Niflib::Vector3> & >(newpoints->GetVertices()));   
		node2_ptr->SetTspaceFlag(16);

		newtriangles->Stripification();
		node2_ptr->SetStripCount(1);
		node2_ptr->SetStrip(0, newtriangles->GetVertices());

		node2_ptr->SetUVSetCount(1);
		node2_ptr->SetUVSet(0, reinterpret_cast<std::vector<Niflib::TexCoord> & >(newpoints->GetUV()));

		int stripcount = node2_ptr->GetStripCount();

		vector<Triangle> newt=node2_ptr->GetTriangles();

		_llLogger()->WriteNextLine(-LOG_INFO, "The (stripe-based) mesh %s has %i triangles and %i vertices",
			filename, newt.size(), newpoints->GetVertices().size());

		if (ninode_ptr) 
			ninode_ptr->AddChild(node_ptr);
	}
	
	if (texname) {
		//optional single texture
		NiTexturingProperty * texture_ptr = new NiTexturingProperty();
		NiTexturingPropertyRef texture = texture_ptr;
		NiSourceTexture * image_ptr = new NiSourceTexture();
		NiSourceTextureRef image = image_ptr;
		image->SetExternalTexture(texname);
		TexDesc tex;
		tex.source = image_ptr;
		texture_ptr->SetTexture(0, tex);
		myavobj->AddProperty(texture);
	}

	if (texset1 || texset2) {
		BSShaderPPLightingProperty * shader_ptr = new BSShaderPPLightingProperty();
		BSShaderPPLightingPropertyRef shader = shader_ptr;
		BSShaderTextureSet * txst_ptr = new BSShaderTextureSet();
		BSShaderTextureSetRef txst = txst_ptr;
		shader->SetTextureSet(txst);
		shader->SetShaderFlags(BSShaderFlags(0x3000));
		shader->SetEnvmapScale(1.0);

		txst->SetTexture(0, texset1); // color
		txst->SetTexture(1, texset2); // normal		
		for (int i=2; i<6; i++) txst->SetTexture(i, "");

		myavobj->AddProperty(shader);
	}

	if (ninode_ptr) {
		if (filename) {
			NifInfo info = NifInfo();
			info.version = 335544325;
			if (_llUtils()->GetValue("_nif_version"))
				sscanf(_llUtils()->GetValue("_nif_version"), "%u", &(info.version));
			WriteNifTree(filename, ninode_ptr, info);
			ninode_ptr = NULL;
		}
	} else if (filename) {
		NifInfo info = NifInfo();
		info.version = 335544325;
		if (_llUtils()->GetValue("_nif_version"))
			sscanf(_llUtils()->GetValue("_nif_version"), "%u", &(info.version));
		WriteNifTree(filename, myavobj, info);
	} else {
		_llLogger()->WriteNextLine(-LOG_WARNING, "No filename, no NiNode...");
	}

	if (_llUtils()->GetValue("_install_dir") && filename) {
		delete filename;
	}

	return 1;
}

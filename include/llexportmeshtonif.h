#ifndef _PLLEXPORTMESHTONIF_H_
#define _PLLEXPORTMESHTONIF_H_

#include "../../lltool/include/llexportmeshtoobj.h"
#include "../../niflib/include/obj/NiNode.h"
#include "../../niflib/include/obj/NiTriStrips.h"
#include "../../niflib/include/obj/NiTriStripsData.h"
#include "../../niflib/include/obj/NiTriShape.h"
#include "../../niflib/include/obj/NiTriShapeData.h"
#include "../../niflib/include/gen/BSSegment.h"
#include "../../niflib/include/niflib.h"

class llExportMeshToNif : public llExportMeshToObj {

protected:

	int useshapes;
	int segmented;
	int makeninode;
	int addgeometrydata;
	int no_uv;

	char *texset1, *texset2;

	float loc_trans_x, loc_trans_y, loc_trans_z;

	static Niflib::NiNode *ninode_ptr;
	vector<Niflib::BSSegment> segments;

public:

	llExportMeshToNif();

	llWorker *Clone() {
		return new llExportMeshToNif(*this);
	}

	int Prepare(void);
	int RegisterOptions(void);
	int Exec(void);

};

#endif

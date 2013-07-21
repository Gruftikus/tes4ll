#ifndef _PLLEXPORTMESHTONIF_H_
#define _PLLEXPORTMESHTONIF_H_

#include "../../lltool/include/llexportmeshtoobj.h"
#include "../../niflib/include/obj/NiNode.h"
#include "../../niflib/include/obj/NiTriStrips.h"
#include "../../niflib/include/obj/NiTriStripsData.h"
#include "../../niflib/include/obj/NiTriShape.h"
#include "../../niflib/include/obj/NiTriShapeData.h"
#include "../../niflib/include/niflib.h"

class llExportMeshToNif : public llExportMeshToObj {

protected:

	int useshapes;

	int makeninode;

	static Niflib::NiNode *ninode_ptr;

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

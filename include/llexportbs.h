#ifndef _PLLEXPORTBS_H_
#define _PLLEXPORTBS_H_

#include "llexportmeshtonif.h"
#include "../../niflib/include/obj/BSMultiBoundNode.h"

class llExportBS : public llExportMeshToNif {

protected:

	int writeonly, makebound, loc_bsnode;
	int flags;

public:

	llExportBS();

	llWorker *Clone() {
		return new llExportBS(*this);
	}

	int Prepare(void);
	int RegisterOptions(void);
	int Exec(void);

};

#endif

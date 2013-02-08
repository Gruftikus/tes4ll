#ifndef _PLLEXPORTMESHTONIF_H_
#define _PLLEXPORTMESHTONIF_H_

#include "../../lltool/include/llexportmeshtoobj.h"

class llExportMeshToNif : public llExportMeshToObj {

protected:

	int useshapes;

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

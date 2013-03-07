#ifndef _PLLIMPORTMAPFROMMODLIST_H_
#define _PLLIMPORTMAPFROMMODLIST_H_

#include "../../lltool/include/llworker.h"
#include "../../lltool/include/llmap.h"
#include "../include/tes4qlod.h"

class llImportMapFromModlist : public llWorker {

protected:

	char     *mapname;
	TES4qLOD *tes4qlod;

public:

	llImportMapFromModlist();

	llWorker * Clone() {
		return new llImportMapFromModlist(*this);
	}

	virtual int Prepare(void);
	virtual int RegisterOptions(void);
	virtual int Exec(void);

};

#endif

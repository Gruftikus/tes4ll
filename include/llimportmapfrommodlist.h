#ifndef _PLLIMPORTMAPFROMMODLIST_H_
#define _PLLIMPORTMAPFROMMODLIST_H_

#include "../../lltool/include/llworker.h"
#include "../../lltool/include/llmap.h"
#include "../include/tes4qlod.h"

class llImportMapFromModlist : public llWorker {

protected:

	char *mapname;
	char *watername;
	int	opt_size_x,
		opt_size_y,
		opt_center;
	int autooffset, readwaterheight;
	TES4qLOD *tes4qlod;

	float quadoffsetx, quadoffsety;
	float x1, y1, x2, y2;

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

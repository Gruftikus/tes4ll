#ifndef _PLLREADLODSETTINGS_H_
#define _PLLREADLODSETTINGS_H_

#include "./../../lltool/include/llworker.h"

class llReadLodSettings : public llWorker {

protected:

	char *filename;
	int   format;

	int unknown1;
	int unknown2;
	int unknown3;
	signed short int x1, y1;
	signed short int x2, y2;
	int unknown4;


public:

	llReadLodSettings();

	llWorker * Clone() {
		return new llReadLodSettings(*this);
	}

	int Prepare(void);
	int RegisterOptions(void);
	int Exec(void);

};

#endif

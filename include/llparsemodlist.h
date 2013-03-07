#ifndef _PLLPARSEMODLIST_H_
#define _PLLPARSEMODLIST_H_

#include <iostream>
#include <windows.h>
#include "../../lltool/include/llworker.h"
#include "../../lltool/include/llutils.h"

class llParseModList : public llWorker {

protected:

	char *esp_list[257]; //list of unsorted esp's
	int num_esp;

	char *esp_list_sorted[256]; //list of sorted esp's
	int num_esp_sorted;

	FILETIME time_list_sorted[256];
	
public:

	//constructor
	llParseModList();

	virtual llWorker * Clone() {
		return new llParseModList(*this);
	}

	virtual int RegisterOptions(void);
	virtual int Exec(void);

};

#endif

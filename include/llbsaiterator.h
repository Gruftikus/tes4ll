#ifndef _PLLBSAITERATOR_H_
#define _PLLBSAITERATOR_H_

#include <windows.h>
#include <tchar.h>

#include "./../../lltool/include/llworker.h"
#include "./../../libbsa/src/libbsa.h"

#include <stdint.h>

class llBsaIterator : public llWorker {

protected:

	char *directory, *regexp;
	char *bsafile;
	int   recursive;

	int init_done;
	int position;
	size_t numAssets;
	char **assetPaths;

	bsa_handle bh;

public:

	llBsaIterator();

	llWorker * Clone() {
		return new llBsaIterator(*this);
	}

	int IsRepeatWorker() {
		return 1;
	}

	int Prepare(void);
	int RegisterOptions(void);
	int Exec(void);

};

#endif

#include "../include/llbsaiterator.h"
#include "./../lltool/include/llutils.h"
#include "./../lltool/include/lllogger.h"

#include <./../boost/boost/filesystem.hpp>

#include <windows.h>
#include <tchar.h>

llBsaIterator::llBsaIterator() : llWorker() {
	SetCommandName("BsaIterator");
	init_done = 0;
}

int llBsaIterator::Prepare(void) {
	if (!llWorker::Prepare()) return 0;

	directory = regexp = bsafile = NULL;
	recursive = 0;

	return 1;
}

int llBsaIterator::RegisterOptions(void) {
	if (!llWorker::RegisterOptions()) return 0;

	RegisterValue("-directory", &directory);
	RegisterValue("-regex",     &regexp);
	RegisterValue("-bsafile",   &bsafile, LLWORKER_OBL_OPTION);
	RegisterFlag("-recursive",  &recursive);

	return 1;
}

int llBsaIterator::Exec(void) {
	if (!llWorker::Exec()) return 0;

	if (!Used("-regex"))  regexp=".*";

	if (!init_done) {
		uint32_t ret = bsa_open(&bh, bsafile);
		if (ret != LIBBSA_OK) {
			_llLogger()->WriteNextLine(-LOG_ERROR, "bsa_open(...) failed! Return code: %i", ret);
			return 0;
		}

		char path[MAX_PATH];
		if (Used("-directory"))
			sprintf(path, "%s\\%s", directory, regexp);
		else
			sprintf(path, "%s", regexp);
		//std::cout << path << std::endl;
		ret = bsa_get_assets(bh, path, &assetPaths, &numAssets);
		if (ret != LIBBSA_OK){
			_llLogger()->WriteNextLine(-LOG_ERROR, "bsa_get_assets(...) failed! Return code: %i", ret);
			return 0;
		}
	
		init_done = 1;
		position  = 0;
	}

	if (!numAssets) {
		_llLogger()->WriteNextLine(-LOG_INFO, "No file found which matches the pattern");
		init_done     = 0;
		repeat_worker = false;
		return 0;
	}

	uint8_t* data;
	size_t   size;
	uint32_t ret = bsa_extract_asset_to_memory (bh, assetPaths[position], &data, &size);
	_llUtils()->DeleteValue("_bsafilename");
	_llUtils()->SetValue("_bsafilename", _llUtils()->NewString(assetPaths[position]));
	//_llLogger()->WriteNextLine(-LOG_INFO, "asset: %s", assetPaths[position]);
	
	if (ret != LIBBSA_OK) {
		_llLogger()->WriteNextLine(-LOG_ERROR, "bsa_extract_asset_to_memory(...) failed! Asset: %s, return code: %i", assetPaths[position], ret);
	} else {

		if (_llUtils()->size) {
			_llUtils()->size = 0;
			delete _llUtils()->data;
		}

		_llUtils()->data = data;
		_llUtils()->size = size;
	}

	position++;

	if (numAssets == position) {
		init_done     = 0;
		repeat_worker = false;
		bsa_close(bh);
	} else 
		repeat_worker = true;

	return 1;
}

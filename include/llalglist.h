#ifndef _PLLALGLIST_H_
#define _PLLALGLIST_H_

#include <vector>

#include "..\include\llalg.h"


class llAlgList {

protected:

	std::vector<llAlg*> alg_list;

public:

	//constructor
	llAlgList();

	void AddAlg(llAlg *_alg) {
		alg_list.resize(alg_list.size()+1);
		alg_list[alg_list.size()-1] = _alg;
	}

	int GetSize(void) {
		return alg_list.size();
	}

	llAlg * GetAlg(int _n) {
		if (_n < GetSize())
			return alg_list[_n];
		return NULL;
	}

};

#endif

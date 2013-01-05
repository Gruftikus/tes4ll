#ifndef _PLLALGLIST_H_
#define _PLLALGLIST_H_

#include <vector>

#include "..\include\llalg.h"


class llAlgCollection {

protected:

	std::vector<llAlg*> alg_list;

public:

	//constructor
	llAlgCollection();

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

	double GetValue(float _x, float _y, double *_value = NULL, double *_ceiling = NULL) {
		if (GetSize() == 0) return 1.;
		if (_ceiling)
			*_ceiling = GetAlg(0)->GetCeiling();
		double value   = GetAlg(0)->GetValue(_x, _y);
		for (int a=1; a<GetSize(); a++) {
			GetAlg(a)->GetValue(_x, _y, &value);
			if (_ceiling)
				GetAlg(a)->GetCeiling(_ceiling);
		}
		if (_value) *_value = value;
		return value;
	}

					
};

class       llAlgList;
llAlgList* _llAlgList();
llAlgList& _fllAlgList();

class llAlgList {

protected:

	std::vector<llAlgCollection*> collections;
	std::vector<char*>            col_name;

public:

	//constructor
	llAlgList();

	int AddAlgCollection(char *_name, llAlgCollection *_col);
	
	int GetSize(void) {
		return collections.size();
	}

	llAlgCollection *GetAlgCollection(char* _name);

};

class llCreateAlgCollection : public llWorker {

protected:

	char *name;

public:

	llCreateAlgCollection();

	llWorker * Clone() {
		return new llCreateAlgCollection(*this);
	}

	virtual int RegisterOptions(void);
	virtual int Init(void);

};

#endif


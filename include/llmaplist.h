#ifndef _PLLMAPLIST_H_
#define _PLLMAPLIST_H_

#include <vector>

#include "..\include\llmap.h"

class       llMapList;
llMapList* _llMapList();
llMapList& _fllMapList();

class llMapList {

protected:

	std::vector<llMap*> map_list;
	std::vector<char*>  map_name;

public:

	//constructor
	llMapList();

	void AddMap(char *_name, llMap *_map) {
		for (int i=0; i<GetSize(); i++) {
			if (_stricmp(_name, map_name[i]) == 0)
				map_list[i] = _map;
		}
		map_list.resize(map_list.size() + 1);
		map_list[map_list.size()-1] = _map;
		map_name.resize(map_name.size() + 1);
		map_name[map_name.size()-1] = _name;
	}

	int GetSize(void) {
		return map_list.size();
	}

	llMap *GetMap(char* _name) {
		for (int i=0; i<GetSize(); i++) {
			if (_stricmp(_name, map_name[i]) == 0)
				return map_list[i];
		}
		return NULL;
	}

};

#endif

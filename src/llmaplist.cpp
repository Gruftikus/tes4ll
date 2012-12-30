//a list of algorithms

#include "..\include\llmaplist.h"
#include <string.h>
#include <stdio.h>

llMapList& _fllMapList()
{
    static llMapList* ans = new llMapList();
    return *ans;
}

llMapList * _llMapList()
{
    return &_fllMapList();
}

//constructor
llMapList::llMapList() {

	map_list.resize(0);
	map_name.resize(0);
	
}



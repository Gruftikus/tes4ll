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


int llMapList::AddMap(char *_name, llMap *_map, llPointList *_points, llTriangleList *_triangles, llPolygonList *_polygons) {
	int size = GetSize();
	for (int i=0; i<size; i++) {
		if (_stricmp(_name, map_name[i]) == 0) {
			_llLogger()->WriteNextLine(-LOG_ERROR,"llMapList::AddMap: map %s already existing", _name);
			return 0;
		}
	}
	map_list.resize(size + 1);
	map_list[size]      = _map;
	map_name.resize(size + 1);
	map_name[size]      = _name;
	point_list.resize(size + 1);
	point_list[size]    = _points;
	triangle_list.resize(size + 1);
	triangle_list[size] = _triangles;
	polygon_list.resize(size + 1);
	polygon_list[size]  = _polygons;
	return 1;
}

int llMapList::AddMap(char *_name, llMap *_map, char *_oldmap) {
	int size = GetSize();
	for (int i=0; i<size; i++) {
		if (_stricmp(_name, map_name[i]) == 0) {
			_llLogger()->WriteNextLine(-LOG_ERROR,"llMapList::AddMap: map %s already existing", _name);
			return 0;
		}
	}
	int num = GetMapNum(_oldmap);
	if (num<0) {
		_llLogger()->WriteNextLine(-LOG_ERROR,"llMapList::AddMap: source map %s not found", _name);
		return 0;
	}
	map_list.resize(size + 1);
	map_list[size]      = _map;
	map_name.resize(size + 1);
	map_name[size]      = _name;
	point_list.resize(size + 1);
	point_list[size]    = point_list[num];
	triangle_list.resize(size + 1);
	triangle_list[size] = triangle_list[num];
	polygon_list.resize(size + 1);
	polygon_list[size]  = polygon_list[num];
	return 1;
}

void llMapList::ExchangeMap(char *_name, llMap *_map) {
	int size = GetSize();
	for (int i=0; i<size; i++) {
		if (_stricmp(_name, map_name[i]) == 0) {
			map_list[i] = _map;
			if (polygon_list[i]) polygon_list[i]->SetMap(_map);
			return;
		}
	}
	_llLogger()->WriteNextLine(-LOG_ERROR,"llMapList::ExchangeMap: map %s not found", _name);
}


llMap *llMapList::GetMap(char* _name) {
	//cout << "GetMap " << _name  << endl;
	for (int i=0; i<GetSize(); i++) {
		if (_stricmp(_name, map_name[i]) == 0)
			return map_list[i];
	}
	return NULL;
}

int llMapList::DeleteMap(char *_name) {
	//deletes the map extry, and deletes also the object(s)
	//if not shared with other entries.

	int num = GetMapNum(_name);
	if (num<0) {
		_llLogger()->WriteNextLine(-LOG_ERROR,"llMapList::DeleteMap: map %s not found", _name);
		return 0;
	}

	llMap          *map       = map_list[num];
	llPointList    *points    = point_list[num];
	llTriangleList *triangles = triangle_list[num];
	llPolygonList  *poly      = polygon_list[num];

	for (int i=0; i<GetSize(); i++) {
		if (i != num) {
			if (map == map_list[i])            map       = NULL;
			if (points == point_list[i])       points    = NULL;
			if (triangles == triangle_list[i]) triangles = NULL;
			if (poly == polygon_list[i])       poly      = NULL;
		}
	}

	if (!map) delete map;
	if (!points) delete points;
	if (!triangles) delete triangles;
	if (!poly) delete poly;

	for (int i=num; i<GetSize()-1; i++) {
		map_name[i]      = map_name[i+1];
		map_list[i]      = map_list[i+1];
		point_list[i]    = point_list[i+1];
		triangle_list[i] = triangle_list[i+1];
		polygon_list[i]  = polygon_list[i+1];
	}

	int size = GetSize();

	map_list.resize(size - 1);
	map_name.resize(size - 1);
	point_list.resize(size - 1);
	triangle_list.resize(size - 1);
	polygon_list.resize(size - 1);

	return 1;

}
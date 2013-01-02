#ifndef _PLLMAPLIST_H_
#define _PLLMAPLIST_H_

#include <vector>

#include "..\include\llmap.h"
#include "..\include\llpolygonlist.h"
#include "..\include\lltrianglelist.h"



class       llMapList;
llMapList* _llMapList();
llMapList& _fllMapList();

class llMapList {

protected:

	std::vector<llMap*>          map_list;
	std::vector<char*>           map_name;
	std::vector<llPointList*>    point_list;
	std::vector<llPolygonList*>  polygon_list;
	std::vector<llTriangleList*> triangle_list;


public:

	//constructor
	llMapList();

	int AddMap(char *_name, llMap *_map, llPointList *_points, llTriangleList *_triangles, llPolygonList *_polygons);
	int AddMap(char *_name, llMap *_map, char *_oldmap);

	void ExchangeMap(char *_name, llMap *_map);
	llMap *GetMap(char* _name);

	int GetMapNum(char* _name) {
		for (int i=0; i<GetSize(); i++) {
			if (_stricmp(_name, map_name[i]) == 0)
				return i;
		}
		return -1;
	}

	int GetSize(void) {
		return map_list.size();
	}

	llPointList *GetPointList(char *_map) {
		int map_p = GetMapNum(_map);
		if (map_p<0) return NULL;
		//cout << map_p << point_list[map_p] << endl;
		return point_list[map_p];
	}
	llTriangleList *GetTriangleList(char *_map) {
		int map_p = GetMapNum(_map);
		if (map_p<0) return NULL;
		//cout << map_p << triangle_list[map_p] << endl;
		return triangle_list[map_p];
	}
	llPolygonList *GetPolygonList(char *_map) {
		int map_p = GetMapNum(_map);
		if (map_p<0) return NULL;
		//cout << map_p << polygon_list[map_p] << endl;
		return polygon_list[map_p];
	}

	float GetX1() {
		llMap * map = GetMap("_heightmap");
		if (!map) return 0.f;
		return map->GetX1();
	};
	float GetY1() {
		llMap * map = GetMap("_heightmap");
		if (!map) return 0.f;
		return map->GetY1();
	};
	float GetX2() {
		llMap * map = GetMap("_heightmap");
		if (!map) return 0.f;
		return map->GetX2();
	};
	float GetY2() {
		llMap * map = GetMap("_heightmap");
		if (!map) return 0.f;
		return map->GetY2();
	};
	float GetWidthXPerRaw() {
		llMap * map = GetMap("_heightmap");
		if (!map) return 0.f;
		return map->GetWidthXPerRaw();
	};
	float GetWidthYPerRaw() {
		llMap * map = GetMap("_heightmap");
		if (!map) return 0.f;
		return map->GetWidthYPerRaw();
	};

};

#endif

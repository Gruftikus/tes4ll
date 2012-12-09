#ifndef _PLLTRIANGLELIST_H_
#define _PLLTRIANGLELIST_H_

#include "../externals/niflib/include/niflib.h"

#include "../include/llpointlist.h"
#include "../include/llmap.h"
#include "../include/lllogger.h"

#define IS_SAME_STRIP 1
#define IS_OTHER_STRIP 2
#define HAS_NO_TRIANGLE -1

#define IS_SINGLETON -2
#define IS_TERMINAL  -3
#define IS_FORK      -4
#define IS_GOOD       0
#define HAS_1SWAP       1
#define HAS_2SWAP       2

using namespace Niflib;


class llTriangle {

private:

	int p1, p2, p3;
	int parity;

	llPointList * points;
	int strip_id;
	int num_neighbor[3],edge_flag[3];
	//lltriangle *tri_neighbor[3];
	int is_terminal, neighbor_pos;
	int done_flag;
	

public:

	//constructor
	llTriangle(int _n1, int _n2, int _n3, llPointList *_r);
	llTriangle();
	int write_flag;
	int touched_flag;
	void SetCorrectParity(void);

	int GetPoint1(void) {return p1;};
	int GetPoint2(void) {return p2;};
	int GetPoint3(void) {return p3;};
	int GetParity(void) {return parity;};
	void SetPoint1(int _p) {p1 = _p;};
	void SetPoint2(int _p) {p2 = _p;};
	void SetPoint3(int _p) {p3 = _p;};
	int Pattern(int _p1, int _p2); //returns binary pattern for p1 and p2

	void SetStripID(int _id) {strip_id = _id;};
	int GetStripID(void) {return strip_id;};

	int SetNeighbor(int _num, int _num_triangle);
	int SetNeighbor(int _num_triangle);
	int GetNextNeighbor(int _sourcetri, int _strip, int _flag, int *_alt, int _debug);
	int GetNeighbor(int _sourcetri, int _flag);
	int GetNeighbor(int _num) {
		return num_neighbor[_num];
	}
	void SetEdgeFlag(int _i, int _flag) {
		edge_flag[_i] = _flag;
		UpdateIsTerminal();
	};
	int GetEdgeFlag(int _i) {
		return edge_flag[_i];
	};
	void FlipEdge(int _nei);

	void UpdateIsTerminal(void) {
		is_terminal=0;
		for (int j=0;j<3;j++) {
			if (edge_flag[j] == IS_SAME_STRIP) {
				is_terminal++;
			}
		}    
		if (is_terminal==0) is_terminal=1;
		else if (is_terminal>1) is_terminal=0;
	};

	int IsTerminal(void) {
		return is_terminal;
	};
	void Reset(void) {
		neighbor_pos=0;
	};

	void Print(void);

	void SetDoneFlag(int _d) {
		done_flag = _d;
	}
	int GetDoneFlag(void) {
		return done_flag;
	}
	int GetNPos(void) {
		return neighbor_pos;
	}

	int draw_flag;

};

class llTriangleList {

private:

	std::vector<llTriangle> v;
	unsigned int counter;
	llPointList * points;
	int next_strip_id;
	vector< unsigned short > length_strip;
	vector< unsigned short > vertices;
	int num_strips, pos_strip,lastflag;

	float x00,x11,y00,y11;
	llLogger * mesg;

public:

	//constructor
	llTriangleList(int _n, llPointList *_x, llLogger *_mesg);

	int AddTriangle(int _p1, int _p2, int _p3);
	void Add2Triangles(int _p1, int _p2, int _p3, int _p4);
	int RemoveTriangle(unsigned int _n);

	llTriangle * GetTriangle(unsigned int _n);
	int GetTriangle(int _p1, int _p2); //returns triangle which shares p1 and p2

	int GetParity(int _n) {return v[_n].GetParity();};

	int GetN(void) {return counter;};
	int GetPoint1(int _n) {return (v[_n].GetPoint1());};
	int GetPoint2(int _n) {return (v[_n].GetPoint2());};
	int GetPoint3(int _n) {return (v[_n].GetPoint3());};


	void DivideAt(bool _atx, float _n,
		llMap *_map); //divide all triangles at x
	void DivideBetween(float _x1, float _y1, float _x2, float _y2, llMap *_map);
	int DivideAtZ(float _z, float _mindist,
		      llMap *_map); //divide all triangles at z
	int SplitFlatTriangles(float _min, float _max, float _z,
		llMap *_map);
	int RemoveBrokenTriangles(llMap *_map);


	void MakeStripifyFlags(void); 
	//update neighbors, and flag them if belonging to same strip

	int GetTriangleQuality(int _trinum, int _flag);
	int GetCommonPoints(int _tri1, int _tri2, int _pointnum);

	void UpdateEdgeFlags(int _numtri);
	void UpdateNeighbors(void);
	int GetTunnel(int _starttri);
	int Stripification(void);

	void Print(void);
	void WritePS(char *_name);
	float ps_x00, ps_x11, ps_y00, ps_y11;

	vector< unsigned short > GetVertices(void) {
		return vertices;
	};
	int AddTriangle(llTriangle *_tri, int _flag);
	int HasLoop(int _tri);
	int HasLoop(int _tri, int _source);
};

#endif

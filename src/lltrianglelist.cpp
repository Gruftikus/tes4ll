#include "..\include\lltrianglelist.h"


llTriangle::llTriangle(int _n1, int _n2, int _n3, llPointList *_r) {

	p1     = _n1;
	p2     = _n2;
	p3     = _n3;
	points = _r;
	parity = points->GetParity(_n1, _n2, _n3);
	strip_id = -1;

	num_neighbor[0]=-1;
	num_neighbor[1]=-1;
	num_neighbor[2]=-1;
	// tri_neighbor[0]=NULL;
	// tri_neighbor[1]=NULL;
	// tri_neighbor[2]=NULL;
	edge_flag[0]=-1;
	edge_flag[1]=-1;
	edge_flag[2]=-1;
	is_terminal=0;
	neighbor_pos=0;
	draw_flag=0;
	done_flag=0;write_flag=1;touched_flag=0;

	if ((p1 == p2) ||  (p3 == p2) || (p3 == p1))
		std::cout << "[Warning] Double points in triangle ctor" << std::endl;
	SetCorrectParity();
}

llTriangle::llTriangle() {
	p1     = -1;
	p2     = -1;
	p3     = -1;
	parity = 3;
}

void llTriangle::Print(void) {
	std::cout << "******* Triangle ********" << std::endl;
	for (int i=0;i<3;i++) {
		std::cout << edge_flag[i] <<  ":" << num_neighbor[i]  << std::endl;
	}
	std::cout << "Points:" << p1  << "," << p2 << "," << p3 << std::endl;
	points->Print(p1);
	points->Print(p2);
	points->Print(p3);

}

int llTriangle::SetNeighbor(int _num, int _num_triangle) {
	if (_num<0 || _num>2) return 0;
	num_neighbor[_num] = _num_triangle;
	return 1;
}

int llTriangle::SetNeighbor(int _num_triangle) {
	int num=0;
	if (num_neighbor[0]>-1) num=1;
	if (num_neighbor[1]>-1) num=2;
	if (num_neighbor[2]>-1) {
		std::cout << "[Error] SetNeighbor: no free slot (used: " <<  num_neighbor[0] << "," << num_neighbor[1] 
		<< "," << num_neighbor[2] << ", new:" <<  _num_triangle <<  ")" << std::endl;
		Print();
		return 0;
	}

	return SetNeighbor(num, _num_triangle);
}



void llTriangle::SetCorrectParity(void) {
	if (points->GetParity(p1, p2, p3) == 1) {
		int swap = p1; p1=p2;p2=swap;
	}
}

int llTriangle::Pattern(int _p1, int _p2) {
	int pattern=0;
	if (_p1 == p1) pattern|=1;
	if (_p1 == p2) pattern|=2;
	if (_p1 == p3) pattern|=4;
	if (_p2 == p1) pattern|=1;
	if (_p2 == p2) pattern|=2;
	if (_p2 == p3) pattern|=4;
	return pattern;
}


int llTriangle::GetNextNeighbor(int _sourcetri, int _strip, int _flag, int *_alt, int _debug) {
	int ret=-1;
	(*_alt)=0;
	int total_alt=0;
	for (int i=0;i<3;i++) {
		if (_debug)
			std::cout << "EF:" << edge_flag[i] << 
			" IF:" << _flag  << " NEI:" << num_neighbor[i]  
		<< " NP:" <<neighbor_pos
			<< std::endl;
		if (edge_flag[i] == _flag) {
			if (_sourcetri != num_neighbor[i]) {
				if ((neighbor_pos == total_alt) && (ret==-1)) {
					ret=num_neighbor[i];
					neighbor_pos++;
				} else if (ret>-1) {
					(*_alt)++;
				}
				total_alt++;
			}
		}
	}
	return ret;
};

int llTriangle::GetNeighbor(int _sourcetri, int _flag) {
	int ret=-1;
	for (int i=0;i<3;i++) {
		if (edge_flag[i] == _flag) {
			if (_sourcetri != num_neighbor[i]) {
				if (ret==-1) {
					ret=num_neighbor[i];
				} else if (ret>-1) {
					return -1; //FATAL: more than one
				}
			}
		}
	}
	return ret;
};

void llTriangle::FlipEdge(int _nei) {
	for (int i=0;i<3;i++) {
		if (num_neighbor[i] == _nei) {
			if (edge_flag[i] == IS_SAME_STRIP)
				edge_flag[i] = IS_OTHER_STRIP;
			else if (edge_flag[i] == IS_OTHER_STRIP)
				edge_flag[i] = IS_SAME_STRIP;
		}
	}
	UpdateIsTerminal();
}

//constructor
llTriangleList::llTriangleList(int _n, llPointList *_x) {

	v.resize(_n + 1);
	counter = 0;
	points  = _x;
	next_strip_id = 1;
	length_strip.resize(1);
	num_strips = 0;
	pos_strip  = 0;
	mesg = _llLogger();
} 

void llTriangleList::Print(void) {
	for (unsigned int i=0;i<counter;i++) {
		std::cout << "Triangle:" << i  << " " << GetTriangle(i)->GetPoint1() 
			<< ":"<<GetTriangle(i)->GetPoint2()<< ":"<< GetTriangle(i)->GetPoint3()   << std::endl;
		GetTriangle(i)->Print();
		std::cout << "*********" << std::endl;
	}
}


void llTriangleList::WritePS(char *_name) {
	fprintf(stderr,"Writing ps file \"%s\"\n",_name);
	FILE *f;    
	if (fopen_s(&f,_name,"w")) {
		fprintf(stderr,"Unable to open ps file \"%s\"\n",_name);
		exit(-1);
	}

	float scale = 500.f/(ps_x11 - ps_x00);
	for (unsigned int j=0;j<counter;j++) {
		llTriangle * tri = GetTriangle(j);

		int c1=GetPoint1(j);
		int c2=GetPoint2(j);
		int c3=GetPoint3(j);
		float x1 = (points->GetX(c1)-ps_x00)*scale+75;
		float y1 = (points->GetY(c1)-ps_y00)*scale+200;
		float z1 = points->GetZ(c1);
		float x2 = (points->GetX(c2)-ps_x00)*scale+75;
		float y2 = (points->GetY(c2)-ps_y00)*scale+200;
		float z2 = points->GetZ(c2);
		float x3 = (points->GetX(c3)-ps_x00)*scale+75;
		float y3 = (points->GetY(c3)-ps_y00)*scale+200;
		float z3 = points->GetZ(c3);

#if 1
		//if (tri->draw_flag) {
		if (z1<=0 && z2<=0 && z3<=0) {
			fprintf(f,"newpath %f %f moveto %f %f lineto %f %f lineto 0.9 setgray fill\n",x1,y1,x2,y2,x3,y3);
		}
#endif
		fprintf(f," 0 setgray \n");


		fprintf(f,"newpath %f %f moveto %f %f lineto 0.1 setlinewidth stroke\n",x1,y1,x2,y2);
		fprintf(f,"newpath %f %f moveto %f %f lineto 0.1 setlinewidth stroke\n",x1,y1,x3,y3);
		fprintf(f,"newpath %f %f moveto %f %f lineto 0.1 setlinewidth stroke\n",x3,y3,x2,y2);

#if 0
		fprintf(f,"/Times-Roman findfont 8 scalefont setfont \n");	
		fprintf(f,"newpath %f %f moveto ",(x1+x2+x3)/3,(y1+y2+y3)/3);
		int q1 = GetTriangleQuality(j,0);
		fprintf(f,"(%i) show\n",q1);

		for (int i=0;i<3;i++) {
			int nei = tri->GetNeighbor(i);
			int neiflag = tri->GetEdgeFlag(i);
			if (nei>=0) {
				c1=GetCommonPoints(j,nei,0);
				c2=GetCommonPoints(j,nei,1);
			} else {
				c1=c2=-1;
			}

			if (c1 != -1 && c2 != -1 && neiflag == IS_OTHER_STRIP) {
				x1 = (points->GetX(c1)-ps_x00)*scale+75;
				y1 = (points->GetY(c1)-ps_y00)*scale+200;
				x2 = (points->GetX(c2)-ps_x00)*scale+75;
				y2 = (points->GetY(c2)-ps_y00)*scale+200;
				fprintf(f,"newpath %f %f moveto %f %f lineto 1.0 setlinewidth stroke\n",x1,y1,x2,y2);
			}

		} 
#endif	
	}
	fclose(f);
}

void llTriangleList::UpdateNeighbors(void) {
	for (unsigned int i=0;i<counter;i++) {
		llTriangle * v = GetTriangle(i);
		//look for neighbor(s)
		int p1=v->GetPoint1();
		int p2=v->GetPoint2();
		int p3=v->GetPoint3();
		int nei1 = GetTriangle(p1, p2);
		int nei2 = GetTriangle(p2, p3);
		int nei3 = GetTriangle(p1, p3);
		if (nei1>-1 && nei1!=i) {
			v->SetNeighbor(nei1);
			GetTriangle(nei1)->SetNeighbor(i);
			UpdateEdgeFlags(nei1);
		}
		if (nei2>-1 && nei2!=i) {
			v->SetNeighbor(nei2);
			GetTriangle(nei2)->SetNeighbor(i);
			UpdateEdgeFlags(nei2);
		}
		if (nei3>-1 && nei3!=i) {
			v->SetNeighbor(nei3);
			GetTriangle(nei3)->SetNeighbor(i);
			UpdateEdgeFlags(nei3);
		}
		UpdateEdgeFlags(i);
	}
}

int llTriangleList::RemoveTriangle(unsigned int n) {
	if (n >= counter) return 0;

	for (unsigned int i = n; i < counter -1; i++) {
		v[i] = v[i+1];
	}
	counter--;
	return 1;
}

int llTriangleList::AddTriangle(int _p1, int _p2, int _p3) {

	if ((_p1 == _p2) ||  (_p3 == _p2) || (_p3 == _p1)) {
		mesg->WriteNextLine(LOG_WARNING,"Triangle with two equal points (maybe a very small one) -> removed");
		mesg->Dump();
#if 0
		std::cout << p1 << ":" << p2 << ":" << p3 << std::endl;
		std::cout << points->GetX(p1) << ":" << points->GetY(p1) << std::endl;
		std::cout << points->GetX(p2) << ":" << points->GetY(p2) << std::endl;
		std::cout << points->GetX(p3) << ":" << points->GetY(p3) << std::endl;
#endif
		return -1;
	}

#if 0
	for (int i=0;i<counter;i++) {
		if (p1 == v[i].GetPoint1() || p1 == v[i].GetPoint2() || p1 == v[i].GetPoint3()) {
			if (p2 == v[i].GetPoint1() || p2 == v[i].GetPoint2() || p2 == v[i].GetPoint3()) {
				if (p3 == v[i].GetPoint1() || p3 == v[i].GetPoint2() || p3 == v[i].GetPoint3()) {
					std::cout << "[Info] Triangle already exists" << std::endl;
					return -1;
				}
			}
		}

	}
#endif

	v[counter] = llTriangle(_p1, _p2, _p3, points);
	v[counter].SetStripID(next_strip_id);
	next_strip_id++;
	counter++;

	if (counter == v.size()) v.resize(counter+100);
	return counter - 1;
}


int llTriangleList::GetTriangleQuality(int _trinum, int _flag) {

	// int p1 = GetPoint1(trinum);
	// int p2 = GetPoint2(trinum);
	// int p3 = GetPoint3(trinum);
	llTriangle * tri = GetTriangle(_trinum);

	// int nei1 = GetTriangle(p1, p2);
	// int nei2 = GetTriangle(p1, p3);
	// int nei3 = GetTriangle(p3, p2);
	int neiflag1 = -1, neiflag2 = -1, neiflag3 = -1;

	int c1=-1, c2, c3, c4, snei1, snei2;

	int num_same_strip = 0;

	for (int i=0;i<3;i++) {
		int nei = tri->GetNeighbor(i);
		int neiflag = tri->GetEdgeFlag(i);
		
		if (neiflag == IS_SAME_STRIP) {
			num_same_strip++;
			if (c1==-1) {
				snei1=nei;
				c1=GetCommonPoints(_trinum,nei,0);
				c2=GetCommonPoints(_trinum,nei,1);
			} else {
				snei2=nei;
				c3=GetCommonPoints(_trinum,nei,0);
				c4=GetCommonPoints(_trinum,nei,1);
			} 
		}

	}
	if ((num_same_strip == 0 || num_same_strip == 1 || num_same_strip == 3) && !_flag) return 0;
	if (num_same_strip == 0 && _flag) return IS_SINGLETON;
	if (num_same_strip == 1) return IS_TERMINAL;
	if (num_same_strip == 3) return IS_FORK;

	//look for common point
	int common=-1;
	if (c1 == c3) common=c1;
	if (c1 == c4) common=c1;
	if (c2 == c3) common=c2;
	if (c2 == c4) common=c2;


	//if common ==-1 ........

	llTriangle * tri1 = GetTriangle(snei1);
	llTriangle * tri2 = GetTriangle(snei2);

	int snei1a = -1;
	if (tri1)
		snei1a = tri1->GetNeighbor(_trinum, IS_SAME_STRIP);
	int snei2a = -1;
	if (tri2)
		snei2a = tri2->GetNeighbor(_trinum, IS_SAME_STRIP);

	//determine number of swaps
	int swaps=0;

	if (snei1a != -1) {
		if (common == GetCommonPoints(snei1a, snei1, 0)) swaps++;
		else if (common == GetCommonPoints(snei1a, snei1, 1)) swaps++;
	}
	if (snei2a != -1) {
		if (common == GetCommonPoints(snei2a, snei2, 0)) swaps++;
		else if (common == GetCommonPoints(snei2a, snei2, 1)) swaps++;
	}

	if (!swaps) return IS_GOOD;
	if (swaps==1) return HAS_1SWAP;
	if (swaps==2) return HAS_2SWAP;

	if (_flag) return -1;
	return 0;

};

int llTriangleList::GetCommonPoints(int _tri1, int _tri2, int _pointnum) {

	for (int i=0; i<3; i++) {
		int p1 = GetPoint1(_tri1);
		if (i==1) p1 = GetPoint2(_tri1);
		else if (i==2) p1 = GetPoint3(_tri1);
		for (int j=0; j<3; j++) {
			int p2 = GetPoint1(_tri2);
			if (j==1) p2 = GetPoint2(_tri2);
			else if (j==2) p2 = GetPoint3(_tri2);	    
			if (p1 == p2) {
				if (!_pointnum) return p1;
				else _pointnum--;
			}
		}
	}
	return -1;
}


void llTriangleList::UpdateEdgeFlags(int _numtri) {

	for (int i=0;i<3;i++) {
		int nei = GetTriangle(_numtri)->GetNeighbor(i);
		if (nei>-1) {
			llTriangle *neit = GetTriangle(nei);
			if (neit->GetStripID() == GetTriangle(_numtri)->GetStripID()) {
				GetTriangle(_numtri)->SetEdgeFlag(i, IS_SAME_STRIP);
			} else {
				GetTriangle(_numtri)->SetEdgeFlag(i, IS_OTHER_STRIP);
			} 
		} else GetTriangle(_numtri)->SetEdgeFlag(i, HAS_NO_TRIANGLE);
	}    
}



llTriangle * llTriangleList::GetTriangle(unsigned int _n) {
	if ((_n<0) || _n>=counter) return NULL;
	return &(v[_n]);
}

int llTriangleList::GetTriangle(int _p1, int _p2) {

	int num = -1, num2=-1;

	for (unsigned int i=0;i<counter;i++) {
		int p1 = v[i].GetPoint1();	
		int p2 = v[i].GetPoint2();
		int p3 = v[i].GetPoint3();

		if ((p1 == _p1 && p2 == _p2) || 
			(p1 == _p2 && p2 == _p1) ||
			(p1 == _p1 && p3 == _p2) || 
			(p1 == _p2 && p3 == _p1) ||
			(p2 == _p1 && p3 == _p2) || 
			(p2 == _p2 && p3 == _p1)) {

				if (num!=-1 && num2 !=-1) {
					mesg->WriteNextLine(LOG_WARNING,"The edge with the points (%i,%i) appeared in triangle %i, %i and %i", _p1, _p2, num, num2, i);
					mesg->Dump();
#if 0				
					GetTriangle(num)->Print();
					GetTriangle(num2)->Print();
					GetTriangle(i)->Print();
#endif
					//exit(-1);
				} else if (num!=-1) num2=num;

				num = i;
		}
	}
	return num;
}

int llTriangleList::HasLoop(int _tri) {
	llTriangle * current = GetTriangle(_tri);
	int l=0;
	for (int i=0;i<3;i++) {
		if (current->GetEdgeFlag(i)==IS_SAME_STRIP) {
			int next = current->GetNeighbor(i);
			l+=HasLoop(next, _tri);
		}
	}
	return l;
}

int llTriangleList::HasLoop(int _tri, int _source) {

	int loop=1;
	int newsource=_tri, nextstrip;
	llTriangle * current = GetTriangle(_tri);
	while (loop) {		
		if (current->IsTerminal()) return 0;

		nextstrip = current->GetNeighbor(_source, IS_SAME_STRIP);
		if (nextstrip==-1) {
			return 1;
		} else {
			if (nextstrip == _tri) {
				return 1;
			}

			_source   = newsource;
			newsource = nextstrip;
			current   = GetTriangle(nextstrip);		    
			if (current->IsTerminal()) return 0;
		}
	}
	return 0;
}	    


#define ERROR -1
#define SINGLETON_TRI 1
#define START_TRI 2
#define END_TRI 3
#define MIDDLE_TRI 4

int llTriangleList::Stripification(void) {
	//Tunneling algorithm of Steward, Porku, etc...

	UpdateNeighbors();
	int make_stripification=1, triangle=0, last_found_triangle=counter-1;
	int terminal=0;
	lastflag=-1;

	mesg->WriteNextLine(LOG_INFO,"Stripification started");
	mesg->Dump();

	while (make_stripification) {
		//get next terminal
		if (!GetTriangle(triangle)) {
			mesg->WriteNextLine(LOG_FATAL,"Triangle: %i not present",triangle);
			mesg->Dump();
			exit(-1);
		}
		if (GetTriangle(triangle)->IsTerminal()) {
			terminal++;
			//Found terminal
			int result = GetTunnel(triangle);
			if (result == 1) {
				//OK
				last_found_triangle=triangle;
			}
		}
		triangle++;
		if (triangle==counter) {
			mesg->WriteNextLine(LOG_INFO,"Terminals processed: %i",terminal);
			mesg->Dump();
			terminal=0;
			triangle=0;
		}
		if (triangle==last_found_triangle) { 
			//1 cycle, failed
			make_stripification=0;
		}
	}

	mesg->WriteNextLine(LOG_INFO,"Now stitch the strips");
	mesg->Dump();

	//now we set the length of the strips
	//and the total stiched strip
	vertices.resize(65536);

	for (unsigned int i=0;i<counter;i++) {
		llTriangle * current = GetTriangle(i);

		if ((current->GetDoneFlag()==0) && current->IsTerminal()) {

			//start a new strip here
			length_strip[num_strips]=0;
			//current->draw_flag=1;
			int nextstrip, loop=1, source=-1, newsource=i, current_i=i;

			if (current->GetNeighbor(source, IS_SAME_STRIP) == -1) {		
				loop=0; //singleton
				AddTriangle(current, SINGLETON_TRI);
			} else {
				AddTriangle(current, START_TRI);
			}
			current->SetDoneFlag(1);

			int added=1;
			while (loop) {	

				nextstrip = current->GetNeighbor(source, IS_SAME_STRIP);
				if (nextstrip==-1) {
					loop=0;
					mesg->WriteNextLine(LOG_ERROR,"Forked strip: %i (Startstrip: %i)",source,i);
					GetTriangle(i)->Print();
					mesg->Dump();
				} else {

					length_strip[num_strips]++;
					GetTriangle(nextstrip)->SetDoneFlag(1);
					if (GetTriangle(nextstrip)->IsTerminal()) {
						length_strip[num_strips]++;
						added=AddTriangle(GetTriangle(nextstrip), END_TRI);
						loop=0;
					} else {
						added=AddTriangle(GetTriangle(nextstrip), MIDDLE_TRI);
					}

					if (nextstrip == i){
						loop=0;
						mesg->WriteNextLine(LOG_ERROR,"Looped strip");
					}

					if (!added) { //try a new endpoint
						current->FlipEdge(nextstrip);
						GetTriangle(nextstrip)->FlipEdge(current_i);
						loop=0;
					} else {
						source=newsource;
						newsource=nextstrip;
						current= GetTriangle(nextstrip);		    
						current_i=nextstrip;
					}		    
				}
			}

			current->SetDoneFlag(1);
			num_strips++;
			length_strip.resize(1+num_strips);
		}
	}

	for (unsigned int i=0;i<counter;i++) {
		llTriangle * current = GetTriangle(i);
		if (current->GetDoneFlag()==0) {
			mesg->WriteNextLine(LOG_ERROR,"Untouched triangle %i",i);
			current->Print();
			mesg->Dump();
		}
	}

	vertices.resize(pos_strip);
	//vertices.resize(pos);
	mesg->WriteNextLine(LOG_INFO,"Strips: %i, trianglepoints: %i",num_strips,pos_strip);
	mesg->Dump();
	return 1;
}


int llTriangleList::AddTriangle(llTriangle *_tri, int _flag) {

	if (pos_strip > 65530) {
		mesg->WriteNextLine(LOG_FATAL,"Too many vertices");
		mesg->Dump();
		return 0;
	}

	//even position
	int v1a = _tri->GetPoint1();
	int v2a = _tri->GetPoint2();
	int v3a = _tri->GetPoint3();

	if ((pos_strip % 2) == 1) {
		v1a = _tri->GetPoint1();
		v3a = _tri->GetPoint2();
		v2a = _tri->GetPoint3();
	}

	if (pos_strip==0) {
		vertices[pos_strip]=v1a;pos_strip++;
		vertices[pos_strip]=v2a;pos_strip++;
		vertices[pos_strip]=v3a;pos_strip++;     
		lastflag = _flag;
		return 1;
	} 

	//read the last steps
	int v1=vertices[pos_strip-3];
	int v2=vertices[pos_strip-2];
	int v3=vertices[pos_strip-1];

	if (_flag == SINGLETON_TRI) {
		vertices[pos_strip]=v3;pos_strip++;
		vertices[pos_strip]=v1a;pos_strip++;
		vertices[pos_strip]=v1a;pos_strip++;
		vertices[pos_strip]=v2a;pos_strip++;
		vertices[pos_strip]=v3a;pos_strip++;
		lastflag=_flag;
		return 1;
	}

	if (_flag == START_TRI) {
		vertices[pos_strip]=v3;pos_strip++;
		vertices[pos_strip]=v1a;pos_strip++;
		vertices[pos_strip]=v1a;pos_strip++;
		vertices[pos_strip]=v2a;pos_strip++;
		vertices[pos_strip]=v3a;pos_strip++;
		lastflag=_flag;
		return 1;
	}

	if ((_flag == MIDDLE_TRI || _flag == END_TRI) && lastflag == START_TRI) {
		//we still can play....
		int c=0;
		while (!((v2 == v1a) && (v3 == v2a) ) && (c < 10)) {
			int swap=v1;
			v1=v2;v2=v3;v3=swap;
			c++;
			if ((c==3) || (c==6)) {
				swap=v1a;
				v1a=v2a;v2a=v3a;v3a=swap;
			}
		}
		if (c==10) {
			mesg->WriteNextLine(LOG_ERROR,"Starting triangles cannot be stitched");
			mesg->Dump();
			//std::cout << v1 << ":" << v2 << ":" << v3 << std::endl;
			//std::cout << v1a << ":" << v2a << ":" << v3a << std::endl;
		} else {
			//write back new order
			if (pos_strip>4) {
				vertices[pos_strip-4]=v1;
			}
			vertices[pos_strip-3]=v1;
			vertices[pos_strip-2]=v2;
			vertices[pos_strip-1]=v3;
			if (_flag == END_TRI) {
				vertices[pos_strip]=v3a;pos_strip++;
				lastflag=_flag;
				return 1;
			}
			vertices[pos_strip]=v3a;pos_strip++;
			lastflag=_flag;
			return 1;
		}
	}

	if ((_flag == MIDDLE_TRI || _flag == END_TRI)) {
		int c=0;
		while ((!((v2 == v1a) && (v3 == v2a) )   
			&&  !((v1 == v2a) && (v3 == v1a) )  
			) 
			&& (c < 3)) {
				int swap=v1a;
				v1a=v2a;v2a=v3a;v3a=swap;
				c++;
		}
		if (c==3) {
			mesg->WriteNextLine(LOG_ERROR,"Following triangles cannot be stitched");
			mesg->WriteNextLine(LOG_ERROR,"%i %i %i",v1,v2,v3);
			mesg->WriteNextLine(LOG_ERROR,"%i %i %i",v1a,v2a,v3a);
			mesg->Dump();
		} else {
			if ((v2 == v1a) && (v3 == v2a)) {
				//this is correct way of a strip
				vertices[pos_strip]=v3a;pos_strip++;
				lastflag=_flag;
				return 1;
			} else {
				vertices[pos_strip] = vertices[pos_strip-1]; //shift 4
				vertices[pos_strip-1] = vertices[pos_strip-3];pos_strip++; //3
				vertices[pos_strip]=v3a;pos_strip++;

				// vertices[pos_strip]=v1a;pos_strip++;
				// vertices[pos_strip]=v2a;pos_strip++;
				// vertices[pos_strip]=v3a;pos_strip++;
				lastflag=_flag;
				return 1;
				//std::cout << "needs swap"  << std::endl;
			}
		}
	}

	lastflag=_flag;
	return 0;
};

int llTriangleList::GetTunnel(int _starttri) {
	llTriangle * start = GetTriangle(_starttri);
	start->draw_flag=1;

	int startstrip=start->GetStripID();
	int llist[1000];
	int local_quality[1000];
	int lalt[1000];
	llist[0] = _starttri;
	for (int i=0;i<1000;i++) local_quality[i]=0;

	int has_found_terminal=0,debug=0;


	int listpos=1,max_depth=1;
	while (has_found_terminal==0) {
		int sourcetri=-1;
		int find_alt_path=0,increase_depth=0;
		//STEP1: find the next neighbor triangle
		//Odd listpos: IS_OTHER_STRIP
		//Even:        IS_SAME_STRIP
		int flag=IS_OTHER_STRIP;
		if ((listpos % 2) == 0) flag=IS_SAME_STRIP;
		llTriangle * current = GetTriangle(llist[listpos-1]);

		if (listpos>1) sourcetri=llist[listpos-2];
getnext:
		local_quality[listpos]=0;
		//if (starttri==3) debug=1;
		llist[listpos]=current->GetNextNeighbor(sourcetri,startstrip,flag,&(lalt[listpos-1]),0);

		//STEP2: we have found another triangle
		if (llist[listpos]>-1) {
			if (debug) std::cout << "Next triangle:" << listpos << std::endl;
			//found something..........
			//we flip the edge between the 2 triangles
			llTriangle * target = GetTriangle(llist[listpos]);

			if (target->GetNPos()) { //already visited
				goto getnext;
			}

			int is_terminal=target->IsTerminal();
			local_quality[listpos] = GetTriangleQuality(llist[listpos],0);
			local_quality[listpos] += GetTriangleQuality(llist[listpos-1],0);
			if (debug) std::cout <<  local_quality  <<  std::endl;
			current->FlipEdge(llist[listpos]);	    
			target->FlipEdge(llist[listpos-1]);

			local_quality[listpos] -= GetTriangleQuality(llist[listpos],0);
			local_quality[listpos] -= GetTriangleQuality(llist[listpos-1],0);
			if (debug) 	std::cout << "quality after flip:" <<   local_quality[listpos]  
			<<  std::endl;

			target->draw_flag=1;
			if (debug) {
				WritePS("bla");
				system("sleep 0.1; okular obj.ps");
			}
			target->draw_flag=0;


			if (flag==IS_SAME_STRIP) { //step one: glue the triangle, step 2: divide it
				if (HasLoop(llist[listpos-1])) {
					current->FlipEdge(llist[listpos]);	    
					target->FlipEdge(llist[listpos-1]);
					goto getnext;
				}
			} 

			//STEP3: check for terminal, if the last flag was OTHER_STRIP
			if (flag==IS_OTHER_STRIP && is_terminal) {			   
				int current_quality=0;
				for (int i=0;i<=listpos;i++) current_quality += local_quality[i];
				if (!HasLoop(llist[listpos]) && (current_quality >= -2)) {
					has_found_terminal=1;
				} 
			}
			if (!has_found_terminal) {
				//let us seek one step further, if we are not at the limit
				if (listpos< max_depth) {
					listpos++;
				} else {
					//if we are at the maximum, we have to undo the last
					//operation, because an alternative path has to be found
					current->FlipEdge(llist[listpos]);	    
					target->FlipEdge(llist[listpos-1]);
					find_alt_path=1;
					increase_depth=1;
					local_quality[listpos]=0;
				}
			}
		} else {
			find_alt_path=1;
		}

		if (find_alt_path) {
			//Still nothing found, let us hope that we find an alternative in the
			//next step
			while ((lalt[listpos-1]==0) && (listpos>1)  ) {
				//if this is not the case, we have to go step by step
				//back and undo the operations we have done
				//until we have an alternative for listpos-1
				listpos--;
				current = GetTriangle(llist[listpos-1]);
				llTriangle * target = GetTriangle(llist[listpos]);

				current->FlipEdge(llist[listpos]);	    
				target->FlipEdge(llist[listpos-1]);

				target->Reset();
			}
			if ((lalt[0]==0) && (listpos==1)) {
				//we are back at the beginning, without success
				current = GetTriangle(llist[listpos-1]);
				llTriangle * target=GetTriangle(llist[listpos]);
				current->Reset();

				//let us try a step deeper
				if (max_depth<32 && (increase_depth)) {
					max_depth++;
				}
				else
					has_found_terminal=ERROR;
			}
		}
	}

	for (int i=0;i<listpos;i++)
		GetTriangle(llist[i])->Reset();
	start->draw_flag=0;
	return has_found_terminal;
}

int llTriangleList::DivideAtZ(float _z_in, float _mindist, llMap *_map) { 
	//divide all triangles at z (not fully tested to the end)
	unsigned old_counter=counter;
	int num_new=0;
	for (unsigned int i=0;i<old_counter;i++) {
		float x[4],y[4],z[4];

		x[1] =  points->GetX(GetPoint1(i));
		x[2] =  points->GetX(GetPoint2(i));
		x[3] =  points->GetX(GetPoint3(i));
		y[1] =  points->GetY(GetPoint1(i));
		y[2] =  points->GetY(GetPoint2(i));
		y[3] =  points->GetY(GetPoint3(i));
		z[1] =  points->GetZ(GetPoint1(i));
		z[2] =  points->GetZ(GetPoint2(i));
		z[3] =  points->GetZ(GetPoint3(i));

		int soliton=0;
		if (z[1]<=_z_in && z[2]>_z_in && z[3]>_z_in) soliton=-1;
		if (z[2]<=_z_in && z[1]>_z_in && z[3]>_z_in) soliton=-2;
		if (z[3]<=_z_in && z[2]>_z_in && z[1]>_z_in) soliton=-3;
		if (z[1]>_z_in && z[2]<=_z_in && z[3]<=_z_in) soliton=1;
		if (z[2]>_z_in && z[1]<=_z_in && z[3]<=_z_in) soliton=2;
		if (z[3]>_z_in && z[2]<=_z_in && z[1]<=_z_in) soliton=3;
		if (soliton==0) continue;
		float other_x[2],other_y[2],other_z[2], single_x, single_y, single_z;

		if (soliton == 1 || soliton == -1) {
			other_x[0]=x[2];
			other_x[1]=x[3];
			other_y[0]=y[2];
			other_y[1]=y[3];
			other_z[0]=z[2];
			other_z[1]=z[3];
			single_x  =x[1];
			single_y  =y[1];
			single_z  =z[1];
		}
		if (soliton == 2 || soliton == -2) {
			other_x[0]=x[1];
			other_x[1]=x[3];
			other_y[0]=y[1];
			other_y[1]=y[3];
			other_z[0]=z[1];
			other_z[1]=z[3];
			single_x  =x[2];
			single_y  =y[2];
			single_z  =z[2];
		}
		if (soliton == 3 || soliton == -3) {
			other_x[0]=x[1];
			other_x[1]=x[2];
			other_y[0]=y[1];
			other_y[1]=y[2];
			other_z[0]=z[1];
			other_z[1]=z[2];
			single_x  =x[3];
			single_y  =y[3];
			single_z  =z[3];
		}
		float dist[2];
		dist[0] = sqrt( pow(other_x[0]-single_x,2) +  pow(other_y[0]-single_y,2) );
		dist[1] = sqrt( pow(other_x[1]-single_x,2) +  pow(other_y[1]-single_y,2) );
		if (!dist[0] || !dist[1]) continue; //avoid nonsense

		float diff_x[2],diff_y[2], start_x[2], start_y[2];

		//this assumes that the singleton is lower:
		diff_x[0] = -(other_x[0]-single_x)/dist[0];
		diff_y[0] = -(other_y[0]-single_y)/dist[0];
		diff_x[1] = -(other_x[1]-single_x)/dist[1];
		diff_y[1] = -(other_y[1]-single_y)/dist[1];
		start_x[0] = other_x[0];
		start_y[0] = other_y[0];
		start_x[1] = other_x[1];
		start_y[1] = other_y[1];

		//if not turn around:
		if (soliton>0) {
			start_x[0] = single_x;
			start_y[0] = single_y;
			start_x[1] = single_x;
			start_y[1] = single_y;
			diff_x[0] = - diff_x[0];
			diff_x[1] = - diff_x[1];
			diff_y[0] = - diff_y[0];
			diff_y[1] = - diff_y[1];
		}

		//now find the cross point. We always go from high to low
		int found[2];
		found[0]=0;
		found[1]=0;


		for (int j=0;j<2;j++) {
			float localdiff=0;
			while (_map->GetZ(start_x[j],start_y[j]) > _z_in && localdiff<(dist[j] - _mindist)) {
				start_x[j] += diff_x[j];
				start_y[j] += diff_y[j];
				localdiff += sqrt(diff_x[j] * diff_x[j] + diff_y[j] * diff_y[j]);
			}
			if (localdiff<(dist[j] - _mindist)) found[j]++;
			if (localdiff<=(_mindist)) found[j]--;

		}
		if (found[0] == 0 && found[1] == 0) {
			continue;  //Nothing
		}
		if ((found[0] + found[1]) == 1) {
			llTriangle * loc = &(v[i]);
#if 1
			if (found[0]) {
				loc->SetPoint1(points->GetPoint(other_x[1],other_y[1]));
				loc->SetPoint2(points->GetPoint(single_x,single_y));
				int point = points->GetPoint(start_x[0],start_y[0]);
				if (point<0)
					point=points->AddPoint(start_x[0], start_y[0], _map->GetZ(start_x[0], start_y[0]));
				int oldpoint = loc->GetPoint3();
				loc->SetPoint3(point);
				loc->SetCorrectParity();

				int point1,point2,point3;
				point1=points->GetPoint(start_x[0],start_y[0]);
				if (point1<0)
					point1=points->AddPoint(start_x[0], start_y[0], _map->GetZ(start_x[0], start_y[0]));
				point2=points->GetPoint(other_x[0],other_y[0]);
				point3=points->GetPoint(other_x[1],other_y[1]);
				if (point1 != point2 && point2 != point3 && point1 != point3)
				AddTriangle(point1,point2,point3);
				else {continue;loc->SetPoint3(oldpoint);}
			} else {
				loc->SetPoint1(points->GetPoint(other_x[0],other_y[0]));
				loc->SetPoint2(points->GetPoint(single_x,single_y));
				int point = points->GetPoint(start_x[1],start_y[1]);
				if (point<0)
					point=points->AddPoint(start_x[1], start_y[1], _map->GetZ(start_x[1],start_y[1]));
				int oldpoint = loc->GetPoint3();
				loc->SetPoint3(point);
				loc->SetCorrectParity();

				int point1,point2,point3;
				point1=points->GetPoint(start_x[1],start_y[1]);
				if (point1<0)
					point1=points->AddPoint(start_x[1], start_y[1], _map->GetZ(start_x[1],start_y[1]));
				point2=points->GetPoint(other_x[0],other_y[0]);
				point3=points->GetPoint(other_x[1],other_y[1]);
				if (point1 != point2 && point2 != point3 && point1 != point3)
				AddTriangle(point1,point2,point3);
				else {continue;loc->SetPoint3(oldpoint);}
			}
#endif
			num_new++;
			continue;
		}
		
		//continue;
		
		if (sqrt((start_x[1]-start_x[0])*(start_x[1]-start_x[0]) + 
			 (start_y[1]-start_y[0])*(start_y[1]-start_y[0]))<=1) continue;

		int point1,point2;
		point1=points->GetPoint(start_x[0],start_y[0]);
		point2=points->GetPoint(start_x[1],start_y[1]);
		if (point1<0)
			point1=points->AddPoint(start_x[0], start_y[0], _map->GetZ(start_x[0],start_y[0]));
		if (point2<0)
			point2=points->AddPoint(start_x[1], start_y[1], _map->GetZ(start_x[1],start_y[1]));
		int orig1,orig2;
		if (point1 == point2) continue;
		if (soliton==1 || soliton==-1) {
		    if (point1 == v[i].GetPoint1() || point2 == v[i].GetPoint1()) continue;
			orig1=v[i].GetPoint2();
			orig2=v[i].GetPoint3();
			if (point1 == orig1 || point1 == orig2 || point2 == orig1 || point2 == orig2) continue;
			v[i].SetPoint2(point1);
			v[i].SetPoint3(point2);
		}
		if (soliton==2 || soliton==-2) {
		    if (point1 == v[i].GetPoint2() || point2 == v[i].GetPoint2()) continue;
			orig1=v[i].GetPoint1();
			orig2=v[i].GetPoint3();
			if (point1 == orig1 || point1 == orig2 || point2 == orig1 || point2 == orig2) continue;
			v[i].SetPoint1(point1);
			v[i].SetPoint3(point2);
		}		
		if (soliton==3 || soliton==-3) {
		    if (point1 == v[i].GetPoint3() || point2 == v[i].GetPoint3()) continue;
			orig1=v[i].GetPoint1();
			orig2=v[i].GetPoint2();
			if (point1 == orig1 || point1 == orig2 || point2 == orig1 || point2 == orig2) continue;
			v[i].SetPoint1(point1);
			v[i].SetPoint2(point2);
		}
		v[i].SetCorrectParity();

		Add2Triangles(point1,point2,orig1,orig2);
		num_new+=2;
	}
	return num_new;
}

void llTriangleList::DivideBetween(float _xx1, float _yy1, float _xx2, float _yy2, llMap *_map) {
	int p1 = points->GetPoint(_xx1, _yy1);
	int p2 = points->GetPoint(_xx2, _yy2);

	if (p1 < 0) {
		mesg->WriteNextLine(LOG_ERROR,"DivideBetween: vertex (%f,%f) not found in vertex list",_xx1, _yy1);
		return;
	}
	if (p2 < 0) {
		mesg->WriteNextLine(LOG_ERROR,"DivideBetween: vertex (%f,%f) not found in vertex list",_xx2, _yy2);
		return;
	}

	unsigned old_counter=counter;
	for (unsigned int i=0;i<old_counter;i++) {

		int num12 = points->GetOverlap(p1,p2,GetPoint1(i),GetPoint2(i));
		int num13 = points->GetOverlap(p1,p2,GetPoint1(i),GetPoint3(i));
		int num23 = points->GetOverlap(p1,p2,GetPoint2(i),GetPoint3(i));

		int num_all = num12 + num13 + num23;

		if (num_all == 1) {
			//GetTriangle(i)->Print();
			float x,y;
			if (num12 == 1) {
				points->GetIntersection(p1,p2,GetPoint1(i),GetPoint2(i),&x,&y);
			}
			if (num23 == 1) {
				points->GetIntersection(p1,p2,GetPoint2(i),GetPoint3(i),&x,&y);
			}
			if (num13 == 1) {
				points->GetIntersection(p1,p2,GetPoint1(i),GetPoint3(i),&x,&y);
			}

			int point1 = points->GetPoint(x,y);
			if (point1<0)
				point1=points->AddPoint(x, y, _map->GetZ(x,y));
			int orig1,orig2;
			if (num23 == 1) {
				orig1=v[i].GetPoint1();
				orig2=v[i].GetPoint2();
				v[i].SetPoint2(point1);
				AddTriangle(orig1, orig2, point1);
			}
			if (num13 == 1) {
				orig1=v[i].GetPoint2();
				orig2=v[i].GetPoint3();
				v[i].SetPoint3(point1);
				AddTriangle(orig1, orig2, point1);
			}
			if (num12 == 1) {
				orig1=v[i].GetPoint3();
				orig2=v[i].GetPoint1();
				v[i].SetPoint1(point1);
				AddTriangle(orig1, orig2, point1);
			}
			v[i].SetCorrectParity();
		}

		if (num_all == 2) {

			float x1,y1,x2,y2;
			if (num12 == 0) {
				points->GetIntersection(p1,p2,GetPoint1(i),GetPoint3(i),&x1,&y1);
				points->GetIntersection(p1,p2,GetPoint2(i),GetPoint3(i),&x2,&y2);
			}
			if (num23 == 0) {
				points->GetIntersection(p1,p2,GetPoint1(i),GetPoint2(i),&x1,&y1);
				points->GetIntersection(p1,p2,GetPoint1(i),GetPoint3(i),&x2,&y2);
			}
			if (num13 == 0) {
				points->GetIntersection(p1,p2,GetPoint1(i),GetPoint2(i),&x1,&y1);
				points->GetIntersection(p1,p2,GetPoint2(i),GetPoint3(i),&x2,&y2);
			}
			int point1 = points->GetPoint(x1,y1);
			if (point1<0)
				point1=points->AddPoint(x1, y1, _map->GetZ(x1,y1));
			int point2 = points->GetPoint(x2,y2);
			if (point2<0)
				point2=points->AddPoint(x2, y2, _map->GetZ(x2,y2));
			int orig1,orig2;
			if (num23 == 0) {
				orig1=v[i].GetPoint2();
				orig2=v[i].GetPoint3();
				v[i].SetPoint2(point1);
				v[i].SetPoint3(point2);
			}
			if (num13 == 0) {
				orig1=v[i].GetPoint1();
				orig2=v[i].GetPoint3();
				v[i].SetPoint1(point1);
				v[i].SetPoint3(point2);
			}		
			if (num12 == 0) {
				orig1=v[i].GetPoint1();
				orig2=v[i].GetPoint2();
				v[i].SetPoint1(point1);
				v[i].SetPoint2(point2);
			}
			v[i].SetCorrectParity();

			if (point1 != point2)
				Add2Triangles(point1,point2,orig1,orig2);
			else {
				int t = AddTriangle(point1,orig1,orig2);
				v[t].SetCorrectParity();
			}
		}
	}
}

void llTriangleList::DivideAt(bool _atx, float _n, llMap *_map) { //divide all triangles at x (or y)

	unsigned old_counter = counter;
	for (unsigned int i=0;i<old_counter;i++) {

		//we have to see if not all points are either
		//on the right or left side
		int myloop=0;
		float x1 =  points->GetX(GetPoint1(i));
		float x2 =  points->GetX(GetPoint2(i));
		float x3 =  points->GetX(GetPoint3(i));
		float y1 =  points->GetY(GetPoint1(i));
		float y2 =  points->GetY(GetPoint2(i));
		float y3 =  points->GetY(GetPoint3(i));

		int left=0;
		int right =0;
		int nearpoint =0;
		float mindist=150;

		if (_atx) {
			if (x1<(_n-1.f)) left++; else if (x1>(_n+1.f)) right++;
			if (x2<(_n-1.f)) left++; else if (x2>(_n+1.f)) right++;
			if (x3<(_n-1.f)) left++; else if (x3>(_n+1.f)) right++;
		} else {
			if (y1<(_n-1.f)) left++; else if (y1>(_n+1.f)) right++;
			if (y2<(_n-1.f)) left++; else if (y2>(_n+1.f)) right++;
			if (y3<(_n-1.f)) left++; else if (y3>(_n+1.f)) right++;
		}


		if (left!=3 && right!=3 && (left && right)) {
			//divide triangle #i

			//which point is the soliton?
			int soliton=0;
			float x1s,x2s,x3s,y1s,y2s,y3s;

			if (_atx) {
				if (left==1 && right ==2) {
					if (x1<_n) {soliton=1;x1s=x1;x2s=x2;x3s=x3;y1s=y1;y2s=y2;y3s=y3;}
					if (x2<_n) {soliton=2;x1s=x2;x2s=x1;x3s=x3;y1s=y2;y2s=y1;y3s=y3;}
					if (x3<_n) {soliton=3;x1s=x3;x2s=x2;x3s=x1;y1s=y3;y2s=y2;y3s=y1;}
				} else if (right==1 && left == 2) {
					if (x1>_n) {soliton=1;x1s=x1;x2s=x2;x3s=x3;y1s=y1;y2s=y2;y3s=y3;}
					if (x2>_n) {soliton=2;x1s=x2;x2s=x1;x3s=x3;y1s=y2;y2s=y1;y3s=y3;}
					if (x3>_n) {soliton=3;x1s=x3;x2s=x2;x3s=x1;y1s=y3;y2s=y2;y3s=y1;}
				} else if (left==1 && right==1) { //one point is on the line
					if (fabs(x1-_n)<1.f) {soliton=-1;x1s=x1;x2s=x2;x3s=x3;y1s=y1;y2s=y2;y3s=y3;}
					if (fabs(x2-_n)<1.f) {soliton=-2;x1s=x2;x2s=x1;x3s=x3;y1s=y2;y2s=y1;y3s=y3;}
					if (fabs(x3-_n)<1.f) {soliton=-3;x1s=x3;x2s=x2;x3s=x1;y1s=y3;y2s=y2;y3s=y1;}
				}
			} else {
				if (left==1 && right ==2) {
					if (y1<_n) {soliton=1;x1s=x1;x2s=x2;x3s=x3;y1s=y1;y2s=y2;y3s=y3;}
					if (y2<_n) {soliton=2;x1s=x2;x2s=x1;x3s=x3;y1s=y2;y2s=y1;y3s=y3;}
					if (y3<_n) {soliton=3;x1s=x3;x2s=x2;x3s=x1;y1s=y3;y2s=y2;y3s=y1;}
				} else if (right==1 && left == 2) {
					if (y1>_n) {soliton=1;x1s=x1;x2s=x2;x3s=x3;y1s=y1;y2s=y2;y3s=y3;}
					if (y2>_n) {soliton=2;x1s=x2;x2s=x1;x3s=x3;y1s=y2;y2s=y1;y3s=y3;}
					if (y3>_n) {soliton=3;x1s=x3;x2s=x2;x3s=x1;y1s=y3;y2s=y2;y3s=y1;}
				} else if (left==1 && right==1) { //one point is on the line
					if (fabs(y1-_n)<1.f) {soliton=-1;x1s=x1;x2s=x2;x3s=x3;y1s=y1;y2s=y2;y3s=y3;}
					if (fabs(y2-_n)<1.f) {soliton=-2;x1s=x2;x2s=x1;x3s=x3;y1s=y2;y2s=y1;y3s=y3;}
					if (fabs(y3-_n)<1.f) {soliton=-3;x1s=x3;x2s=x2;x3s=x1;y1s=y3;y2s=y2;y3s=y1;}
				}
			}

			if (soliton == 0) std::cout << "[Warning] DivideAt: Soliton not found, left=" << left << ", right=" << right <<  std::endl;

			if (soliton<0) { //one point (1) on the line
				float slope1,new1;
				if (_atx) {
					slope1=(y3s-y2s)/(x3s-x2s);
					new1=y2s+slope1*(_n-x2s);
				} else {
					slope1=(x3s-x2s)/(y3s-y2s);
					new1=x2s+slope1*(_n-y2s);
				}
				int point1;
				if (_atx) {
					point1=points->GetPoint(_n,new1);
					if (point1<0)
						point1=points->AddPoint(_n, new1, _map->GetZ(_n,new1));
				} else {
					point1=points->GetPoint(new1,_n);
					if (point1<0)
						point1=points->AddPoint(new1, _n, _map->GetZ(new1,_n));
				}

				int orig1,orig2;
				if (soliton==-1) {
					orig1=v[i].GetPoint1();
					orig2=v[i].GetPoint2();
					v[i].SetPoint2(point1);
					AddTriangle(orig1, orig2, point1);
				} else if (soliton==-2) {
					orig1=v[i].GetPoint2();
					orig2=v[i].GetPoint3();
					v[i].SetPoint3(point1);
					AddTriangle(orig1, orig2, point1);
				} else if (soliton==-3) {
					orig1=v[i].GetPoint3();
					orig2=v[i].GetPoint1();
					v[i].SetPoint1(point1);
					AddTriangle(orig1, orig2, point1);
				}
				v[i].SetCorrectParity();
			} else if (soliton>0) {

				float slope1,slope2,new1,new2;

				if (_atx) {
					slope1=(y2s-y1s)/(x2s-x1s);
					slope2=(y3s-y1s)/(x3s-x1s);
					new1=y1s+slope1*(_n-x1s);
					new2=y1s+slope2*(_n-x1s);
				} else {
					slope1=(x2s-x1s)/(y2s-y1s);
					slope2=(x3s-x1s)/(y3s-y1s);
					new1=x1s+slope1*(_n-y1s);
					new2=x1s+slope2*(_n-y1s);
				}

				int point1,point2;

				if (_atx) {
					point1=points->GetPoint(_n,new1);
					point2=points->GetPoint(_n,new2);
					if (point1<0)
						point1=points->AddPoint(_n, new1, _map->GetZ(_n,new1));
					if (point2<0)
						point2=points->AddPoint(_n, new2, _map->GetZ(_n,new2));
				} else {
					point1=points->GetPoint(new1,_n);
					point2=points->GetPoint(new2,_n);
					if (point1<0)
						point1=points->AddPoint(new1, _n, _map->GetZ(new1,_n));
					if (point2<0)
						point2=points->AddPoint(new2, _n, _map->GetZ(new2,_n));
				}

				int orig1,orig2;
				if (soliton==1) {
					orig1=v[i].GetPoint2();
					orig2=v[i].GetPoint3();
					v[i].SetPoint2(point1);
					v[i].SetPoint3(point2);
				} else if (soliton==2) {
					orig1=v[i].GetPoint1();
					orig2=v[i].GetPoint3();
					v[i].SetPoint1(point1);
					v[i].SetPoint3(point2);
				} else if (soliton==3) {
					orig1=v[i].GetPoint1();
					orig2=v[i].GetPoint2();
					v[i].SetPoint1(point1);
					v[i].SetPoint2(point2);
				}
				v[i].SetCorrectParity();

				if (point1 != point2)
					Add2Triangles(point1,point2,orig1,orig2);
				else {
					int t = AddTriangle(point1,orig1,orig2);
					v[t].SetCorrectParity();
				}
			}
		}
	}
}

int llTriangleList::SplitFlatTriangles(float _min, float _max, float z, llMap *_map) {
	int num=0;
	int old_counter=counter;
	for (int i=0;i<old_counter;i++) {
		v[i].write_flag=1;
		int orig1=v[i].GetPoint1();
		int orig2=v[i].GetPoint2();
		int orig3=v[i].GetPoint3();
		float z1=points->GetZ(orig1);
		float z2=points->GetZ(orig2);
		float z3=points->GetZ(orig3);
		int flag1=points->GetFlag(orig1);
		int flag2=points->GetFlag(orig2);
		int flag3=points->GetFlag(orig3);
		int num_flags=0;
		if (flag1) num_flags++;
		if (flag2) num_flags++;
		if (flag3) num_flags++;
		int direct_connection = ((num_flags>1) && ((flag1 && (flag1 == -flag2)) || (flag1 && (flag1 == -flag3)) || (flag2 && (flag2 == -flag3)) ));
		direct_connection |= ((num_flags == 3) && (flag1 == flag2) && (flag1 == flag3));
		//direct_connection =0;
		int nummin=0;
		if (z1>_min && z1<_max) nummin++;
		if (z2>_min && z2<_max) nummin++;
		if (z3>_min && z3<_max) nummin++;
		if ((nummin>2)  || direct_connection  ) {
			//mean under water?
			float x1=points->GetX(orig1);
			float x2=points->GetX(orig2);
			float x3=points->GetX(orig3);
			float y1=points->GetY(orig1);
			float y2=points->GetY(orig2);
			float y3=points->GetY(orig3);

			float meanx = (x1+x2+x3)/3;
			float meany = (y1+y2+y3)/3;
			float meanz = _map->GetZ(meanx,meany);

			if ((meanz < z && z<_min) || (meanz > z && z>_max) || direct_connection) {
				num++;
				v[i].write_flag=1;
				int soliton=orig1;
				float newx1=(x1+x2)/2;
				float newx2=(x1+x3)/2;
				float newy1=(y1+y2)/2;
				float newy2=(y1+y3)/2;
				float dist12=(x1-x2)*(x1-x2)+(y1-y2)*(y1-y2);
				float dist23=(x2-x3)*(x2-x3)+(y2-y3)*(y2-y3);
				float dist31=(x3-x1)*(x3-x1)+(y3-y1)*(y3-y1);
				if (dist12<dist23 && dist12<dist31) {
					soliton=orig3;
					newx1=(x3+x2)/2;
					newx2=(x3+x1)/2;
					newy1=(y3+y2)/2;
					newy2=(y3+y1)/2;

				}
				else if (dist31<dist12 && dist31<dist23) {
					soliton=orig2;
					newx1=(x2+x3)/2;
					newx2=(x2+x1)/2;
					newy1=(y2+y3)/2;
					newy2=(y2+y1)/2;		    
				}
#if 1
				int point1=points->GetPoint(newx1,newy1);
				int point2=points->GetPoint(newx2,newy2);
				if (point1<0)
					point1=points->AddPoint(newx1, newy1, _map->GetZ(newx1,newy1));
				if (point2<0)
					point2=points->AddPoint(newx2, newy2, _map->GetZ(newx2,newy2));
				if (soliton==orig1) {
					v[i].SetPoint2(point1);
					v[i].SetPoint3(point2);
					Add2Triangles(point1,point2,orig2,orig3);
					llTriangle * tri1 = GetTriangle(GetTriangle(orig1, orig2));
					if (tri1) tri1->touched_flag=tri1->Pattern(orig1, orig2);
					llTriangle * tri2 = GetTriangle(GetTriangle(orig1, orig3));
					if (tri2) tri2->touched_flag=tri2->Pattern(orig1, orig3);
				}
				if (soliton==orig2) {
					v[i].SetPoint1(point1);
					v[i].SetPoint3(point2);
					Add2Triangles(point1,point2,orig1,orig3);
					llTriangle * tri1 = GetTriangle(GetTriangle(orig2, orig1));
					if (tri1) tri1->touched_flag=tri1->Pattern(orig2, orig1);
					llTriangle * tri2 = GetTriangle(GetTriangle(orig2, orig3));
					if (tri2) tri2->touched_flag=tri2->Pattern(orig2, orig3);		    
				}
				if (soliton==orig3) {
					v[i].SetPoint1(point1);
					v[i].SetPoint2(point2);
					Add2Triangles(point1,point2,orig1,orig2);
					llTriangle * tri1 = GetTriangle(GetTriangle(orig3, orig1));
					if (tri1) tri1->touched_flag=tri1->Pattern(orig3, orig1);
					llTriangle * tri2 = GetTriangle(GetTriangle(orig3, orig2));
					if (tri2) tri2->touched_flag=tri2->Pattern(orig3, orig2);		   
				}
				v[i].SetCorrectParity();
				v[i].touched_flag=0;
#endif

			}
		}

	}

	return num;
}

int llTriangleList::RemoveBrokenTriangles(llMap *_map) {
	int num=0, done=0;
	unsigned int counter_old=counter;
	for (unsigned int i=0;i<counter_old;i++) {

		int orig1=v[i].GetPoint1();
		int orig2=v[i].GetPoint2();
		int orig3=v[i].GetPoint3();
		float x1=points->GetX(orig1);
		float x2=points->GetX(orig2);
		float x3=points->GetX(orig3);
		float y1=points->GetY(orig1);
		float y2=points->GetY(orig2);
		float y3=points->GetY(orig3);

		float newx3=(x1+x2)/2;
		float newy3=(y1+y2)/2;
		//		int point3=points->GetPoint(newx3,newy3); //BUGBUG
		float newx5=(x1+x3)/2;
		float newy5=(y1+y3)/2;
		//		int point5=points->GetPoint(newx5,newy5);
		float newx6=(x2+x3)/2;
		float newy6=(y2+y3)/2;
		//		int point6=points->GetPoint(newx6,newy6);

#if 0
		if (point3>=0) v[i].touched_flag = 3;
		if (point5>=0) v[i].touched_flag = 5;
		if (point6>=0) v[i].touched_flag = 6;
#endif

		if (v[i].touched_flag) {
			done++;

			if ((v[i].touched_flag & 3) == 3)  { //1,2
				int point3=points->GetPoint(newx3,newy3);
				if (point3<0) {
					point3=points->AddPoint(newx3, newy3, _map->GetZ(newx3,newy3));					
					std::cout << "Strange... point not found" <<  endl;
				}
				AddTriangle(point3,v[i].GetPoint3(),v[i].GetPoint2());
				v[i].SetPoint2(point3);
				num++;
			}
			if ((v[i].touched_flag & 5) == 5)  { //1,3			
				int point5=points->GetPoint(newx5,newy5);
				if (point5<0) {
					point5=points->AddPoint(newx5, newy5, _map->GetZ(newx5,newy5));					
					std::cout << "Strange... point not found" <<  endl;
				}
				AddTriangle(point5,v[i].GetPoint2(),v[i].GetPoint3());
				v[i].SetPoint3(point5);
				num++;
			}
			if ((v[i].touched_flag & 6) == 6)  { //2,3
				int point6=points->GetPoint(newx6,newy6);
				if (point6<0) {
					point6=points->AddPoint(newx6, newy6, _map->GetZ(newx6,newy6));					
					std::cout << "Strange... point not found" <<  endl;
				}
				AddTriangle(point6,v[i].GetPoint1(),v[i].GetPoint3());
				v[i].SetPoint3(point6);
				num++;
			}
			v[i].SetCorrectParity();
			v[i].touched_flag=0;
		}
	}
	return num;
};

void llTriangleList::Add2Triangles(int _p1, int _p2, int _p3, int _p4) {
	if (points->GetOverlap(_p1,_p2,_p3,_p4)) {
		int t1 = AddTriangle(_p1,_p2,_p3);
		int t2 = AddTriangle(_p1,_p2,_p4);
		if (t1>-1) v[t1].SetCorrectParity();
		if (t2>-1) v[t2].SetCorrectParity();
	} else if (points->GetOverlap(_p1,_p3,_p2,_p4)) {
		int t1 = AddTriangle(_p1,_p3,_p2);
		int t2 = AddTriangle(_p1,_p3,_p4);
		if (t1>-1) v[t1].SetCorrectParity();
		if (t2>-1) v[t2].SetCorrectParity();
	} else if (points->GetOverlap(_p1,_p4,_p2,_p3)) {
		int t1 = AddTriangle(_p1,_p4,_p3);
		int t2 = AddTriangle(_p1,_p4,_p2);
		if (t1>-1) v[t1].SetCorrectParity();
		if (t2>-1) v[t2].SetCorrectParity();
	} 
}

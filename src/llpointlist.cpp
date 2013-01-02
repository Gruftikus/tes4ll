#include "..\include\llpointlist.h"
#include <math.h>


//constructor
llPointList::llPointList(int n, llQuadList * _quads) {

	Resize(n + 1);
	counter=0;
	quads = _quads;

} 

int llPointList::AddPoint(float _x, float _y, float _z) {

	if (quads) { 
		if (!quads->AddPoint(_x, _y, counter)) return -1;
	}

	v[counter] = Vector3(floor(_x+.5f), floor(_y+.5f), _z);
	float tex_x = (_x-tex00x)/(tex11x-tex00x);
	float tex_y = (_y-tex00y)/(tex11y-tex00y);
	uv[counter] = TexCoord(tex_x,tex_y);
	active[counter] = 1;
	tmp_active[counter] = 1;
	flag[counter] = 0;

	counter++;

	if (counter == v.size()) {
		Resize(counter+1000);
	}
	return counter-1;
}

int llPointList::GetPoint(float _x, float _y) {
	if (quads) {
		llQuad *quad = quads->GetQuad(_x, _y);
		if (quad) return quad->GetPoint(_x, _y);
		return -1;
	} else {
		for (unsigned int i=0; i<counter;i++) {
			if (fabs(GetX(i) - _x)<1.f && fabs(GetY(i) - _y)<1.f) return i;
		}
	}
	return -1;
}

int llPointList::GetPoint(float _x, float _y, float _z) {
	for (unsigned int i=0; i<counter;i++) {
		if (fabs(GetX(i) - _x)<1.f && fabs(GetY(i) - _y)<1.f && fabs(GetZ(i) - _z)<1.f) return i;
	}
	return -1;
}


float llPointList::GetMinDistance(float _x, float _y, float _radius, llQuad *_quad) {
	float min=1E10;
	if (_quad) {
		_quad->GetMinDistance(&min, _x, _y, _radius);
	} else if (!quads || _radius < 0.f) {
		//slow method
		for (unsigned int i=0; i<counter;i++) {
			float minnew = (_x - GetX(i))*(_x - GetX(i)) + (_y - GetY(i))*(_y - GetY(i));
			if (minnew<min) min=minnew;
		}
	} else {
		//quadtree nearest neighbor search
		for (unsigned int i=0; i<quads->GetNumQuads(); i++) {
			llQuad *quad = quads->GetQuad(i);
			if (quad) {
				quad->GetMinDistance(&min, _x, _y, _radius);
			}
		}
	}

	return sqrt(min);
}

int  llPointList::GetClosestPoint(float _x, float _y) {
	float min=1E10;
	int num=-1;
	for (unsigned int i=0; i<counter;i++) {
		float minnew = (_x - GetX(i))*(_x - GetX(i)) + (_y - GetY(i))*(_y - GetY(i));
		if (minnew<min) {min=minnew;num=i;}
	}
	return num;
}

float llPointList::GetMinDistanceGrid(float _x, float _y, int _flag) {
	//flag=0: all, =1: x, =2:y


	float mingrid=float(floor(_x/4096.f)*4096);
	float maxgrid=mingrid+4096.f;

	float min=4096.f;
	if (_flag ==0 || _flag == 1) {
		min=fabs(_x - mingrid);
		if (fabs(maxgrid - _x) < min)  min=fabs(maxgrid - _x);
	}

	mingrid=float(floor(_y/4096.f)*4096);
	maxgrid=mingrid+4096.f;

	if (_flag == 0 || _flag == 2) {
		if (fabs(_y-mingrid) < min)  min=fabs(_y-mingrid);
		if (fabs(maxgrid-_y) < min)  min=fabs(maxgrid-_y);
	}
	return min;
}

int llPointList::GetOverlap(int _p1,int _p2, int _p3,int _p4) {

	//if both lines are equal, or they form a triangle, this is OK

	if ((_p1 == _p3) || (_p1 == _p4) || (_p2 == _p3) || (_p2 == _p4)) return 0;

	float s1_x, s1_y, s2_x, s2_y; 
	s1_x = GetX(_p2) - GetX(_p1);     
	s1_y = GetY(_p2) - GetY(_p1);     
	s2_x = GetX(_p4) - GetX(_p3);
	s2_y = GetY(_p4) - GetY(_p3);

	float s, t; 
	s = (-s1_y * (GetX(_p1) - GetX(_p3)) + s1_x * 
		(GetY(_p1) - GetY(_p3))) / (-s2_x * s1_y + s1_x * s2_y); 
	t = ( s2_x * (GetY(_p1) - GetY(_p3)) - s2_y * 
		(GetX(_p1) - GetX(_p3))) / (-s2_x * s1_y + s1_x * s2_y); 
	if (s >= 0 && s <= 1 && t >= 0 && t <= 1) 
	{ 
		// Collision detected 
		return 1; 
	} 
	return 0; // No collision 
}


int llPointList::GetIntersection(int _p1, int _p2, int _p3, 
	int _p4, float *_x, float *_y) {

	float s1_x, s1_y, s2_x, s2_y; 
	s1_x = GetX(_p2) - GetX(_p1);     
	s1_y = GetY(_p2) - GetY(_p1);     
	s2_x = GetX(_p4) - GetX(_p3);
	s2_y = GetY(_p4) - GetY(_p3);

	float x1=GetX(_p1); 
	float x2=GetX(_p3); 
	float y1=GetY(_p1); 
	float y2=GetY(_p3); 

	float s2 = (y1*s1_x - y2*s1_x + x2*s1_y - x1*s1_y)/(s2_y*s1_x - s2_x*s1_y);
	*_x = x2 + s2*s2_x;
	*_y = y2 + s2*s2_y;
	
	return 0;
}

int llPointList::GetParity(int _p1, int _p2, int _p3) {

	double x1 = GetX(_p1) - GetX(_p2);
	double x2 = GetX(_p2) - GetX(_p3);
	double y1 = GetY(_p1) - GetY(_p2);
	double y2 = GetY(_p2) - GetY(_p3);

	//cross product
	double cross = x1*y2 - x2*y1;

	if (cross<0) return 1;
	if (cross>0) return 2;
	return 0;

}


float llPointList::CalculateAngle(float _v1x, float _v1y, float _v1z, float _v2x, float _v2y, float _v2z) {
	//Tool function which calculates the angle between 2 vectors v1 and v2
	float l1 = sqrt(_v1x*_v1x + _v1y*_v1y + _v1z*_v1z);
	float l2 = sqrt(_v2x*_v2x + _v2y*_v2y + _v2z*_v2z);

	if (!l1 || !l2) return 0;

	_v1x/=l1;_v1y/=l1;_v1z/=l1;
	_v2x/=l2;_v2y/=l2;_v2z/=l2;

	float c = _v1x*_v2x + _v1y*_v2y + _v1z*_v2z;
	if (c<-1.f || c>1.f) return 0;

	float a = fabs(float(acos(c)));
	
	return a;
};

int llPointList::GetClosestPointOnLine(float _sx, float _sy, float _sz, float _vx, float _vy, float _vz,
	float _ix, float _iy, float _iz, float *_t, float *_px, float *_py, float *_pz) {
		//get the closest point which is on the line defined with the starting point at (s)
		//and the vector (v) to the point (i)
		//returns the point (p) and the fraction t of the vector (v)
		*_t = (_vx*_ix - _vx*_sx + _vy*_iy - _vy*_sy + _vz*_iz - _vz*_sz)/
			(_vx*_vx + _vy*_vy + _vz*_vz);
		*_px = *_t * _vx + _sx;
		*_py = *_t * _vy + _sy;
		*_pz = *_t * _vz + _sz;
		return 1;
}

int llPointList::VectorIntersectsWithTriangle(float _x, float _y, float _z, float _vx, float _vy, float _vz, 
	float _x1, float _y1, float _z1, float _x2, float _y2, float _z2, float _x3, float _y3, float _z3, 
	float *_s, float *_u, float *_v) {
		//This function evaluates, if a given vector (vx,vy,vz), starting at (x,y,z) intersects with the triangle plane, which is formed
		//by the points 1,2,3
		//The return value "s" is the fraction of the vector length until the intersection point,
		//"u" and "v" are the coordinates on the triangle plane, defined by the vectors (1->2) and (1->3)

		//Transformation
		
		_x1 -= _x; _y1 -= _y; _z1 -= _z;
		_x2 -= _x; _y2 -= _y; _z2 -= _z;
		_x3 -= _x; _y3 -= _y; _z3 -= _z;

		float vx2=_vx;
		float vy2=_vy;
		float vz2=_vz;
		float x12=_x1;
		float x22=_x2;
		float x32=_x3;
		float y12=_y1;
		float y22=_y2;
		float y32=_y3;
		float z12=_z1;
		float z22=_z2;
		float z32=_z3;

		//rotate such that vector is pointing to z-axis
		float len = sqrt(_vx*_vx + _vy*_vy);
		if (len) {
			float phi = asin(_vy/len);
			if (_vx < 0) phi = 3.14159265f - phi;
			if (phi!=0) {
				float cos_phi = cos(-phi);
				float sin_phi = sin(-phi);
				vx2 = _vx*cos_phi - _vy*sin_phi;
				vy2 = _vx*sin_phi + _vy*cos_phi;
				x12 = _x1*cos_phi - _y1*sin_phi;
				y12 = _x1*sin_phi + _y1*cos_phi;
				_x1=x12; _y1=y12;
				x22 = _x2*cos_phi - _y2*sin_phi;
				y22 = _x2*sin_phi + _y2*cos_phi;
				_x2=x22; _y2=y22;
				x32 = _x3*cos_phi - _y3*sin_phi;
				y32 = _x3*sin_phi + _y3*cos_phi;
				_x3=x32; _y3=y32;
				_vx=vx2; _vy=vy2;
			}
		}
		//now vector is in x/z-plane
		len = sqrt(_vx*_vx + _vz*_vz);
		if (len) {
			float theta = asin(_vx/len);
			if (_vz < 0) theta = 3.14159265f - theta;
			if (theta!=0) {
				float cos_theta = cos(theta);
				float sin_theta = sin(theta);
				vx2 = _vx*cos_theta - _vz*sin_theta;
				vz2 = _vx*sin_theta + _vz*cos_theta;
				x12 = _x1*cos_theta - _z1*sin_theta;
				z12 = _x1*sin_theta + _z1*cos_theta;
				_x1=x12; _z1=z12;
				x22 = _x2*cos_theta - _z2*sin_theta;
				z22 = _x2*sin_theta + _z2*cos_theta;
				_x2=x22; _z2=z22;
				x32 = _x3*cos_theta - _z3*sin_theta;
				z32 = _x3*sin_theta + _z3*cos_theta;
				_x3=x32; _z3=z32;
				_vx=vx2; _vz=vz2;
			}
		}

		if (_vz < 0) {
			_vz = -_vz;
			_x1 = -_x1;
			_x2 = -_x2;
			_x3 = -_x3;
			_z1 = -_z1;
			_z2 = -_z2;
			_z3 = -_z3;
		}

		*_v = (_x1*(_y2-_y1) / (_x2-_x1) - _y1) * (_x1-_x2) / 
			( (_y2-_y1)*(_x3-_x1) + (_x1-_x2)*(_y3-_y1) );
		*_u = (_x1 + (_x3-_x1)* *_v) / (_x1 - _x2);

		*_s = (_z1 + (_z2-_z1)* *_u + (_z3 - _z1)* *_v) / _vz;

		if ((*_u > 0) && (*_v > 0) && ((*_u+*_v)<1) && (*_s > 0) && (*_s < 1)) {
			return 1;
		}

		return 0;
}

#include <iostream>
#include <math.h>

#include <windows.h>
#include <direct.h>
#include <winreg.h>


//#include <windowsx.h>
//#include <d3d9.h>

#include "../externals/niflib/include/obj/NiNode.h"
#include "../externals/niflib/include/obj/NiTriStrips.h"
#include "../externals/niflib/include/obj/NiTriStripsData.h"
#include "../externals/niflib/include/obj/NiTriShape.h"
#include "../externals/niflib/include/obj/NiTriShapeData.h"
#include "../externals/niflib/include/niflib.h"
#include "../externals/niflib/include/obj/NiTexturingProperty.h"
#include "../externals/niflib/include/obj/NiSourceTexture.h"


#define REAL double

#include "..\externals\triangle\triangle.h"

#include "..\include\llutils.h"
#include "..\include\llmap.h"
#include "..\include\llpointlist.h"
#include "..\include\llpolygonlist.h"
#include "..\include\lltrianglelist.h"
#include "..\include\llcommands.h"
#include "..\include\llalg.h"
#include "..\include\llalgconst.h"
#include "..\include\llalgfirst.h"
#include "..\include\llalgsecond.h"
#include "..\include\llalgslope.h"
#include "..\include\llalgstripe.h"
#include "..\include\llalgradial.h"
#include "..\include\llalgpeakfinder.h"
#include "..\include\llquadlist.h"

llMap * heightmap = NULL;
int x_cell=0, y_cell=0;
int npoints = 1;

#define USE_TES4QLOD

#define FROMTES4LL
#ifdef USE_TES4QLOD
#include "..\merged\tes4qlod.c"
#endif

int calltes4qlodwrapper(char *_command, llCommands *_batch, llUtils *_utils) {
	char *mycommand = new char[strlen(_command)+1];
	strcpy(mycommand,_command);
	int argc = 1;
	char *argv[128];
	argv[0] = mycommand;
	bool found_quot =0;
	for (unsigned int i=0; i<strlen(_command)-1; i++) {
		if (mycommand[i]==' ' && !found_quot) {
			mycommand[i]='\0';
			argv[argc] = mycommand+i+1;			
			argc++;
		} else if (mycommand[i]=='\"' ) found_quot = !found_quot;
	}
	for (int i=0;i<argc;i++) {
		_utils->StripSpaces(&(argv[i]));
		_utils->StripQuot(&(argv[i]));
	}
#ifdef USE_TES4QLOD
	if (strlen(_batch->install_dir)>0)
		opt_install_dir = _batch->install_dir;
	return calltes4qlod(argc, argv);
#endif
	return 0;
}


//The bmp reader was taken from Paul Bourkes examples:
//http://paulbourke.net/dataformats/bmp/parse.c
typedef struct {
   unsigned short int type;                 /* Magic identifier            */
   unsigned int size;                       /* File size in bytes          */
   unsigned short int reserved1, reserved2;
   unsigned int offset;                     /* Offset to image data, bytes */
} HEADER;
typedef struct {
   unsigned int size;               /* Header size in bytes      */
   int width,height;                /* Width and height of image */
   unsigned short int planes;       /* Number of colour planes   */
   unsigned short int bits;         /* Bits per pixel            */
   unsigned int compression;        /* Compression type          */
   unsigned int imagesize;          /* Image size in bytes       */
   int xresolution,yresolution;     /* Pixels per meter          */
   unsigned int ncolours;           /* Number of colours         */
   unsigned int importantcolours;   /* Important colours         */
} INFOHEADER;
typedef struct {
   unsigned char r,g,b,junk;
} COLOURINDEX;

int ReadUShort(FILE *fptr,short unsigned *n,int swap)
{
   unsigned char *cptr,tmp;

   if (fread(n,2,1,fptr) != 1)
       return(0);
   if (swap) {
       cptr = (unsigned char *)n;
       tmp = cptr[0];
       cptr[0] = cptr[1];
       cptr[1] =tmp;
   }
   return(1);
}

/*
   Write a possibly byte swapped unsigned short integer
*/
int WriteUShort(FILE *fptr,short unsigned n,int swap)
{
   unsigned char *cptr,tmp;

	if (!swap) {
   	if (fwrite(&n,2,1,fptr) != 1)
      	return(FALSE);
   } else {
      cptr = (unsigned char *)(&n);
      tmp = cptr[0];
      cptr[0] = cptr[1];
      cptr[1] =tmp;
      if (fwrite(&n,2,1,fptr) != 1)
         return(FALSE);
   }
   return(TRUE);
}


int ReadUInt(FILE *fptr,unsigned int *n,int swap)
{
   unsigned char *cptr,tmp;

   if (fread(n,4,1,fptr) != 1)
       return(0);
   if (swap) {
       cptr = (unsigned char *)n;
       tmp = cptr[0];
       cptr[0] = cptr[3];
       cptr[3] = tmp;
       tmp = cptr[1];
       cptr[1] = cptr[2];
       cptr[2] = tmp;
   }
   return(1);
}

int ReadInt(FILE *fptr, int *n,int swap)
{
   unsigned char *cptr,tmp;

   if (fread(n,4,1,fptr) != 1)
       return(0);
   if (swap) {
       cptr = (unsigned char *)n;
       tmp = cptr[0];
       cptr[0] = cptr[3];
       cptr[3] = tmp;
       tmp = cptr[1];
       cptr[1] = cptr[2];
       cptr[2] = tmp;
   }

   return(1);
}

int WriteInt(FILE *fptr,int n,int swap)
{
   unsigned char *cptr,tmp;

	if (!swap) {
   	if (fwrite(&n,4,1,fptr) != 1)
      	return(FALSE);
   } else {
      cptr = (unsigned char *)(&n);
      tmp = cptr[0];
      cptr[0] = cptr[3];
      cptr[3] = tmp;
      tmp = cptr[1];
      cptr[1] = cptr[2];
      cptr[2] = tmp;
      if (fwrite(&n,4,1,fptr) != 1)
         return(FALSE);
   }
   return(TRUE);
}

/*
   Write a possibly byte swapped unsigned integer
*/
int WriteUInt(FILE *fptr,unsigned int n,int swap)
{
   unsigned char *cptr,tmp;

   if (!swap) {
      if (fwrite(&n,4,1,fptr) != 1)
         return(FALSE);
   } else {
      cptr = (unsigned char *)(&n);
      tmp = cptr[0];
      cptr[0] = cptr[3];
      cptr[3] = tmp;
      tmp = cptr[1];
      cptr[1] = cptr[2];
      cptr[2] = tmp;
      if (fwrite(&n,4,1,fptr) != 1)
         return(FALSE);
   }
   return(TRUE);
}

void usage(void) {

	_llLogger()->WriteNextLine(LOG_INFO,"Usage: tes4ll [-x xpos -y ypos] -b batchfile -f \"flags,flags,flags,...\" -w worldspace [heightmap.bmp]");
	_llLogger()->WriteNextLine(LOG_INFO,"       x/ypos: x/y position of the lower left cell");
	_llLogger()->WriteNextLine(LOG_INFO,"       batchfile: the name of the batch script");
	_llLogger()->WriteNextLine(LOG_INFO,"       flags: the flags which are propagated to the batch script");
	_llLogger()->WriteNextLine(LOG_INFO,"       heightmap.bmp: the heighmap in 32 bit");
	_llLogger()->Dump();

}

void DumpExit(void) {		
	_llLogger()->Dump();
	exit(-1);
	//while (1);
}


int WriteBMP(llMap *_heightmap,
	float _x00, float _x11, float _y00, float _y11, char *_filename, llCommands *_batch, int _compress = 1) {

		int bmp_done=0;
		for (float y=_y11; y>_y00; y-=128/heightmap->GetScaling() ) {
			for (float x=_x00; x<_x11; x+=128/heightmap->GetScaling()) {
				float height = float(heightmap->GetZ(x,y));
				if (height>0) bmp_done=1;
			}
		}
		if (!bmp_done) {
			_llLogger()->WriteNextLine(LOG_INFO,"The bmp '%s' seems to be under water -> skipped compression, and deleted it",_filename);
			return 0;
		}
		_llLogger()->WriteNextLine(LOG_INFO,"Generating %s",_filename);
		_llLogger()->Dump();

		FILE *fptr;

		if (fopen_s(&fptr,_filename,"wb")) {
			_llLogger()->WriteNextLine(LOG_FATAL,"Unable to open BMP file '%s'\n",_filename);
			DumpExit();
		}

		INFOHEADER infoheader;
		infoheader.width = int(_batch->bmpscaling * heightmap->GetScaling() * (_x11-_x00)/128.f);
		infoheader.height= int(_batch->bmpscaling * heightmap->GetScaling() * (_y11-_y00)/128.f);
		infoheader.compression=0;
		infoheader.size=40;  
		infoheader.planes=1;       /* Number of colour planes   */
		if (_batch->writenormalmap)
			infoheader.bits=24;         /* Bits per pixel            */
		else
			infoheader.bits=32;
		if (_batch->writenormalmap)
			infoheader.imagesize=3*infoheader.width*infoheader.height;          /* Image size in bytes       */
		else
			infoheader.imagesize=4*infoheader.width*infoheader.height;
		infoheader.xresolution=100;
		infoheader.yresolution=100;     /* Pixels per meter          */
		infoheader.ncolours=0;           /* Number of colours         */
		infoheader.importantcolours;   /* Important colours         */

		HEADER header;
		header.type='M'*256+'B';
		header.size=14+40+infoheader.imagesize;
		header.reserved1=header.reserved2=0;
		WriteUShort(fptr,header.type,0);
		WriteUInt(fptr,header.size,0);
	    WriteUShort(fptr,header.reserved1,0);
		WriteUShort(fptr,header.reserved2,0);
		header.offset=14+40;
		WriteUInt(fptr,header.offset,0);
		
		/* Read and check the information header */
		if (fwrite(&infoheader,sizeof(INFOHEADER),1,fptr) != 1) {
			_llLogger()->WriteNextLine(LOG_FATAL,"Failed to write BMP info header");
			DumpExit();
		}

		float val=0.f;
		float overdrawing = _batch->overdrawing;

		if (_batch->lodshadows) {
			_llLogger()->WriteNextLine(LOG_INFO,"Adding fake LOD shadows, 'north flip boost=%f'", _batch->overdrawing);
		} else if (_batch->writenormalmap)
			_llLogger()->WriteNextLine(LOG_INFO,"Contrast boost=%f", _batch->overdrawing);
		_llLogger()->Dump();

		if (_batch->lodshadows) overdrawing=1.;

		for (float y=_y11; y>_y00; y-=(128.f/float(heightmap->GetScaling()))/_batch->bmpscaling) {
			for (float x=_x00; x<_x11; x+=(128.f/float(heightmap->GetScaling()))/_batch->bmpscaling) {

				float height = float(heightmap->GetZ(x,y));
				
#if 1
				float st_array[3];
				/*************************************/

				if (_batch->lodshadows) {
					for (int v=0;v<3;v++) {
						float directx=-1.f;
						float directy=0.f;
						if (v==1) directx=1;
						if (v==2) {directx=0;directy=-1;}
						float cur_x = x+directx*128*10;
						float cur_y = y+directy*128*10;
						float steigung=0.f;

steigungs_loop:
						float cur_steigung = 0; 
						if (v<2) 
							cur_steigung = float(directx)*(float(heightmap->GetZ(cur_x,cur_y))-height)/(float(cur_x-x));
						else
							//cur_steigung = (float(directy)*(float(heightmap->GetZ(cur_x,cur_y))-height)/(float(cur_y-y)) + (float(directy))*0.5 ); //minus sun at noon
							cur_steigung = (float(directy)*(float(heightmap->GetZ(cur_x,cur_y))-height)/(float(cur_y-y)) );

						if (cur_steigung > steigung) steigung = cur_steigung;
						int resolution=2;
						int stepsize = 8/resolution;
						if ((directx*(cur_x-x) + directy*(cur_y-y))<700*128) stepsize = 128/resolution;
						if ((directx*(cur_x-x) + directy*(cur_y-y))<512*128) stepsize = 32/resolution;
						if ((directx*(cur_x-x) + directy*(cur_y-y))<256*128) stepsize = 16/resolution;					
						cur_x += stepsize*directx*(128);
						cur_y += stepsize*directy*(128);

						if ((directx*(cur_x-x) + directy*(cur_y-y))<1024*128) goto steigungs_loop; 
						st_array[v] = steigung;
					}

				}
				float tilt=0.;

				float tilt_morning=st_array[1];
				float tilt_evening=-st_array[0];
				//tilt=st_array[1]-st_array[0];


				/*************************************/
#endif
				//float tilt=0;
				//calculate the normal vector
				float n_x00 = (heightmap->GetZ(x+128/heightmap->GetScaling(),y) - heightmap->GetZ(x,y)) / (128.f/(heightmap->GetScaling()*overdrawing));
				float n_y00 = (heightmap->GetZ(x,y+128/heightmap->GetScaling()) - heightmap->GetZ(x,y)) / (128.f/(heightmap->GetScaling()*overdrawing));
				float n_x11 = (heightmap->GetZ(x+128/heightmap->GetScaling(),y+128/heightmap->GetScaling()) - heightmap->GetZ(x,y+128/heightmap->GetScaling())) / (128.f/(heightmap->GetScaling()*overdrawing));
				float n_y11 = (heightmap->GetZ(x+128/heightmap->GetScaling(),y+128/heightmap->GetScaling()) - heightmap->GetZ(x+128/heightmap->GetScaling(),y)) / (128.f/(heightmap->GetScaling()*overdrawing));

				//Shadowmap:
				//float n_x00 = 0;
				//float n_y00 = tilt;
				//float n_x11 = 0;
				//float n_y11 = tilt;
				float n_x = (n_x00 + n_x11)/2.f;
				float n_y = (n_y00 + n_y11)/2.f;

				if (_batch->lodshadows) {
					tilt = tilt_morning+tilt_evening;
					n_x+=tilt;
					n_y-=sqrt(st_array[0]*st_array[1]) * _batch->overdrawing;
					if (n_y > -st_array[2]) n_y = -st_array[2];
				}
								
				float norm = sqrt(n_x*n_x + n_y*n_y + 1.f);
				float n_x1a = ((-n_x/norm)*127.f*1.0f+128.f);
				float n_y1a = ((-n_y/norm)*127.f*1.0f+128.f);
				float n_z1a = ((1.f/norm)*127.f*1.0f+128.f);
				
				float angle=0.;
				
				unsigned int n_x1 = unsigned int(n_x1a);
				unsigned int n_y1 = unsigned int(n_y1a*cos(angle) + n_z1a*sin(angle));
				unsigned int n_z1 = unsigned int(-sin(angle)*n_y1a + n_z1a*cos(angle));

				if (_batch->writeheightmap) {
					val=heightmap->GetZ(x,y);
					WriteUInt(fptr,unsigned int(val),0);
				} else {
					fwrite(&n_z1,1,1,fptr);
					fwrite(&n_y1,1,1,fptr);
					fwrite(&n_x1,1,1,fptr);
				}
			}
		}

		fclose(fptr);
		_llLogger()->Dump();

		if (strlen(_batch->dds_tool) > 1 && _compress) {
			char command[1000];
			sprintf_s(command,1000,"%s %s \n", _batch->dds_tool, _filename);
			_llLogger()->WriteNextLine(LOG_INFO,"Executing '%s %s'", _batch->dds_tool, _filename);
			_llLogger()->Dump();
			FILE *tes = _popen(command,"rt");
			char c; 
			if (tes==NULL) {
				_llLogger()->WriteNextLine(LOG_ERROR,"Error calling '%s'",_batch->dds_tool);
			} else {
				int nn=0;
				do {
					c = fgetc (tes);
					if ((isprint(c) || isspace(c)) && nn==0)
						cout << c;
					else if (isprint(c) || isspace(c)) nn++;
					else nn--;

				} while (c != EOF);
				fclose (tes);
			}
		} else {
			return 0;
		}
		_llLogger()->Dump();
		return 1;
}

//*************************************************************************

void WriteNif(llPointList *_points, llTriangleList *_triangles, llMap *_heightmap,
	      float _x00, float _x11, float _y00, float _y11, char *_filename, llCommands *_batch,
	      char *_texname = NULL, int _ps = 0, int _createpedestals = 0, int _useshapes = 0) {

	
    using namespace Niflib;
    float min=44444444,lowestz=999999;

#if 1
    _triangles->DivideAt(true, _x00, _heightmap);    
    _triangles->DivideAt(true, _x11, _heightmap);  
    _triangles->DivideAt(false,_y00, _heightmap);    
    _triangles->DivideAt(false,_y11, _heightmap);  
#endif
    _points->ClearSecondaryList();
	//pointliste kopieren, matching alt->neu
	llPointList * newpoints = new llPointList(_points->GetN(), NULL);
	
	newpoints->SetTexAnchor(_x00, _y00, _x11, _y11);
	newpoints->ClearSecondaryList();
	for (int i=0; i<_points->GetN(); i++) {
		if (_points->GetX(i)>= (_x00) && _points->GetX(i)<= (_x11) &&
			_points->GetY(i)>= (_y00) && _points->GetY(i)<= (_y11)) {

				float d = newpoints->GetMinDistance(_points->GetX(i), _points->GetY(i));
				if (d<min) min=d;
				float z=heightmap->GetZ(_points->GetX(i), _points->GetY(i));
				int newp = newpoints->AddPoint(_points->GetX(i), _points->GetY(i),z);
				if (z<lowestz) lowestz=z;
				_points->SetSecondary(i,newp);
		}
	}

	if (!newpoints->GetN()) {
		_llLogger()->WriteNextLine(LOG_WARNING,"The mesh %s is empty and was therefore not written",_filename);
	    delete newpoints;
	    return;
	}
    
    
	//copy all triangles which have all 3 points in newpoints
	llTriangleList * newtriangles = new llTriangleList(newpoints->GetN(), newpoints);
	
	if (lowestz>0) lowestz=0;
	else lowestz-=100;

	newtriangles->ps_x00 = _x00;
	newtriangles->ps_x11 = _x11;
	newtriangles->ps_y00 = _y00;
	newtriangles->ps_y11 = _y11;
	for (int i=0; i<_triangles->GetN(); i++) {

		int old1 = _triangles->GetPoint1(i);
		int old2 = _triangles->GetPoint2(i);
		int old3 = _triangles->GetPoint3(i);
		int new1 = _points->GetSecondary(old1);
		int new2 = _points->GetSecondary(old2);
		int new3 = _points->GetSecondary(old3);

		if (new1 > -1 && new2 > -1 && new3 > -1 && _triangles->GetTriangle(i)->write_flag) {
			newtriangles->AddTriangle(new1, new2, new3);

			//add optional pedestals
			if (_createpedestals) {
				int p1=-1,p2=-1,opt=-1;
				if (fabs(newpoints->GetX(new1)-_x00)<1.f && fabs(newpoints->GetX(new2)-_x00)<1.f) {p1=new1;p2=new2;opt=0;}
				if (fabs(newpoints->GetX(new1)-_x00)<1.f && fabs(newpoints->GetX(new3)-_x00)<1.f) {p1=new1;p2=new3;opt=0;}
				if (fabs(newpoints->GetX(new2)-_x00)<1.f && fabs(newpoints->GetX(new3)-_x00)<1.f) {p1=new2;p2=new3;opt=0;}
				if (fabs(newpoints->GetX(new1)-_x11)<1.f && fabs(newpoints->GetX(new2)-_x11)<1.f) {p1=new1;p2=new2;opt=1;}
				if (fabs(newpoints->GetX(new1)-_x11)<1.f && fabs(newpoints->GetX(new3)-_x11)<1.f) {p1=new1;p2=new3;opt=1;}
				if (fabs(newpoints->GetX(new2)-_x11)<1.f && fabs(newpoints->GetX(new3)-_x11)<1.f) {p1=new2;p2=new3;opt=1;}

				if (p1>-1) {
					int p3=newpoints->GetPoint(newpoints->GetX(p1),newpoints->GetY(p1), lowestz);
					int p4=newpoints->GetPoint(newpoints->GetX(p2),newpoints->GetY(p2), lowestz);
					if (p3<0) {
						p3 = newpoints->AddPoint(newpoints->GetX(p1),newpoints->GetY(p1), lowestz);
					}
					if (p4<0) {
						p4 = newpoints->AddPoint(newpoints->GetX(p2),newpoints->GetY(p2), lowestz);
					}
					if (opt==0 && newpoints->GetY(p1)>newpoints->GetY(p2)) {
						newtriangles->AddTriangle(p1,p3,p4);
						newtriangles->AddTriangle(p2,p1,p4);
					}
					if (opt==0 && newpoints->GetY(p1)<newpoints->GetY(p2)) {
						newtriangles->AddTriangle(p1,p4,p3);
						newtriangles->AddTriangle(p1,p2,p4);
					}
					if (opt==1 && newpoints->GetY(p1)>newpoints->GetY(p2)) {
						newtriangles->AddTriangle(p1,p4,p3);
						newtriangles->AddTriangle(p1,p2,p4);
					}
					if (opt==1 && newpoints->GetY(p1)<newpoints->GetY(p2)) {
						newtriangles->AddTriangle(p1,p3,p4);
						newtriangles->AddTriangle(p2,p1,p4);
					}
				}

				if (fabs(newpoints->GetY(new1)-_y00)<1.f && fabs(newpoints->GetY(new2)-_y00)<1.f) {p1=new1;p2=new2;opt=2;}
				if (fabs(newpoints->GetY(new1)-_y00)<1.f && fabs(newpoints->GetY(new3)-_y00)<1.f) {p1=new1;p2=new3;opt=2;}
				if (fabs(newpoints->GetY(new2)-_y00)<1.f && fabs(newpoints->GetY(new3)-_y00)<1.f) {p1=new2;p2=new3;opt=2;}
				if (fabs(newpoints->GetY(new1)-_y11)<1.f && fabs(newpoints->GetY(new2)-_y11)<1.f) {p1=new1;p2=new2;opt=3;}
				if (fabs(newpoints->GetY(new1)-_y11)<1.f && fabs(newpoints->GetY(new3)-_y11)<1.f) {p1=new1;p2=new3;opt=3;}
				if (fabs(newpoints->GetY(new2)-_y11)<1.f && fabs(newpoints->GetY(new3)-_y11)<1.f) {p1=new2;p2=new3;opt=3;}

				if (p1>-1) {
					int p3=newpoints->GetPoint(newpoints->GetX(p1),newpoints->GetY(p1), lowestz);
					int p4=newpoints->GetPoint(newpoints->GetX(p2),newpoints->GetY(p2), lowestz);
					if (p3<0) {
						p3 = newpoints->AddPoint(newpoints->GetX(p1),newpoints->GetY(p1), lowestz);
					}
					if (p4<0) {
						p4 = newpoints->AddPoint(newpoints->GetX(p2),newpoints->GetY(p2), lowestz);
					}
					
					if (opt==2 && newpoints->GetX(p1)<newpoints->GetX(p2)) {
						newtriangles->AddTriangle(p1,p3,p4);
						newtriangles->AddTriangle(p2,p1,p4);
					}
					if (opt==2 && newpoints->GetX(p1)>newpoints->GetX(p2)) {
						newtriangles->AddTriangle(p1,p4,p3);
						newtriangles->AddTriangle(p1,p2,p4);
					}
					if (opt==3 && newpoints->GetX(p1)<newpoints->GetX(p2)) {
						newtriangles->AddTriangle(p1,p4,p3);
						newtriangles->AddTriangle(p1,p2,p4);
					}
					if (opt==3 && newpoints->GetX(p1)>newpoints->GetX(p2)) {
						newtriangles->AddTriangle(p1,p3,p4);
						newtriangles->AddTriangle(p2,p1,p4);
					}
				}
			}
		}
	}


	int num_triangles = newtriangles->GetN();
    std::vector<Triangle> t(num_triangles);
	newpoints->Resize();
	newpoints->Translation(_batch->trans_x, _batch->trans_y, _batch->trans_z);

	if (_useshapes) {

		NiTriShape* node_ptr = new NiTriShape;
		NiTriShapeRef node = node_ptr;

		NiTriShapeData * node2_ptr = new NiTriShapeData();
		NiTriShapeDataRef node2 = node2_ptr;
		node_ptr->SetData(node2);
		node_ptr->SetFlags(14);

		node2_ptr->SetVertices(newpoints->GetVertices());   
		node2_ptr->SetTspaceFlag(16);

		for (int i=0;i<num_triangles;i++) {
			t[i].v1 = newtriangles->GetPoint1(i);
			t[i].v2 = newtriangles->GetPoint2(i);
			t[i].v3 = newtriangles->GetPoint3(i);
		}

		node2_ptr->SetTriangles(t);
		node2_ptr->SetUVSetCount(1);
		node2_ptr->SetUVSet(0,newpoints->GetUV());

		if (_texname) {
			//optional textures
			NiTexturingProperty * texture_ptr = new NiTexturingProperty();
			NiTexturingPropertyRef texture = texture_ptr;
			NiSourceTexture * image_ptr = new NiSourceTexture();
			NiSourceTextureRef image = image_ptr;
			image->SetExternalTexture(_texname);
			TexDesc tex;
			tex.source = image_ptr;
			texture_ptr->SetTexture(0,tex);
			node_ptr->AddProperty(texture);
		}

		//int stripcount = node2_ptr->GetStripCount();

		vector<Triangle> newt=node2_ptr->GetTriangles();

		_llLogger()->WriteNextLine(LOG_INFO, "The (shape-based) mesh %s has %i triangles and %i vertices",
			_filename, newt.size(), newpoints->GetVertices().size());

		NifInfo info = NifInfo();
		info.version = 335544325;

		WriteNifTree(_filename, node, info);

	} else {

		NiTriStrips* node_ptr = new NiTriStrips;
		NiTriStripsRef node = node_ptr;

		NiTriStripsData * node2_ptr = new NiTriStripsData();
		NiTriStripsDataRef node2 = node2_ptr;
		node_ptr->SetData(node2);
		node_ptr->SetFlags(14);

		node2_ptr->SetVertices(newpoints->GetVertices());   
		node2_ptr->SetTspaceFlag(16);

		newtriangles->Stripification();
		node2_ptr->SetStripCount(1);
		node2_ptr->SetStrip(0,newtriangles->GetVertices());

		node2_ptr->SetUVSetCount(1);
		node2_ptr->SetUVSet(0,newpoints->GetUV());

		if (_texname) {
			//optional textures
			NiTexturingProperty * texture_ptr = new NiTexturingProperty();
			NiTexturingPropertyRef texture = texture_ptr;
			NiSourceTexture * image_ptr = new NiSourceTexture();
			NiSourceTextureRef image = image_ptr;
			image->SetExternalTexture(_texname);
			TexDesc tex;
			tex.source = image_ptr;
			texture_ptr->SetTexture(0,tex);
			node_ptr->AddProperty(texture);
		}

		int stripcount = node2_ptr->GetStripCount();

		vector<Triangle> newt=node2_ptr->GetTriangles();

		_llLogger()->WriteNextLine(LOG_INFO, "The (shape-based) mesh %s has %i triangles and %i vertices",
			_filename, newt.size(), newpoints->GetVertices().size());

		NifInfo info = NifInfo();
		info.version = 335544325;

		WriteNifTree(_filename, node, info);
	}

	if (_ps) {
		char newfilename[1000];
		sprintf_s(newfilename,1000,"%s.ps",_filename);
		newtriangles->WritePS(newfilename);
	}

	delete newpoints;
	delete newtriangles;

}


//************************************************************************************

int main(int argc, char **argv) {

    FILE *fptr;
    
	int calltesanwynn = 0;
	//char *worldspace="Tamriel";
	
	llLogger   *mesg  = _llLogger();
	llUtils    *utils = _llUtils();
	llCommands *batch = new llCommands();
	utils->SetValue("_worldspace","Tamriel");

    std::cout << "Landscape LOD generator" << std::endl;
	std::cout << "Written by gruftikus@texnexus" << std::endl;
	std::cout << "V4.10, 29.12.2012" << std::endl;
    std::cout << "***********************" << std::endl;

	//open registry
	HKEY keyHandle;
    char rgValue [1024];
//    char fnlRes [1024];
    DWORD size1;
    DWORD Type;

	char *esp_list[257]; //list of unsorted esp's
	int num_esp=0;
	FILE *fesplist=NULL;    
	
	esp_list[num_esp] = new char[1000];

	char *esp_list_sorted[256]; //list of sorted esp's
	char *list_string = NULL;
	FILETIME time_list_sorted[256];
	int num_esp_sorted=0;

    //******************
    //read the arguments
    //******************

	if (argc<2) {
		usage();
		DumpExit();
	}

    char * bmpfilename = argv[argc-1];
    char * batchname = NULL;
	llMap *water = NULL;
	llMap *flow  = NULL;
	llMap *negative_flow = NULL;
	llMap *garbage= NULL;

	for (int i=1; i<(argc-1); i++) {

		if (strcmp(argv[i],"-f")==0) {
			//flagliste holen
			char *ptr;          
			char *saveptr1 = NULL;
			ptr = strtok_int(argv[i+1], ',', &saveptr1);
			
			while(ptr != NULL) {
				char *my_flag_list=new char[strlen(ptr)+2];
				strcpy_s(my_flag_list,strlen(ptr)+1,ptr);
				ptr = strtok_int(NULL, ',', &saveptr1);
				utils->AddFlag(my_flag_list);

				mesg->WriteNextLine(LOG_INFO,"Flag: %s",my_flag_list);
			}
		}

		if (strcmp(argv[i],"-t")==0) {
			calltesanwynn = 1;
		}

		if (strcmp(argv[i],"-l")==0) {
			list_string = argv[i+1];
			mesg->WriteNextLine(LOG_INFO,"Mod list: %s",list_string);
		}

		if (strcmp(argv[i],"-x")==0) {
			sscanf_s(argv[i+1],"%i",&x_cell);
			mesg->WriteNextLine(LOG_INFO,"x corner: %i",x_cell);
		}

		if (strcmp(argv[i],"-y")==0) {
			sscanf_s(argv[i+1],"%i",&y_cell);
			mesg->WriteNextLine(LOG_INFO,"y corner: %i",y_cell);
		}

		if (strcmp(argv[i],"-b")==0) {
			batchname = argv[i+1];
			mesg->WriteNextLine(LOG_INFO,"Batch file: %s",batchname);
		}

		if (strcmp(argv[i],"-w")==0) {
			utils->SetValue("_worldspace",argv[i+1]);
			mesg->WriteNextLine(LOG_INFO,"Worldspace: %s",utils->GetValue("_worldspace"));
		}
	}

	//add NULL objects
	llQuadList    *quads    = NULL;
	llPolygonList *polygons = NULL;

	int x1=0,y1=0,x2=0,y2=0;
	//Quadlist
    int quadx1,quady1,quadx2,quady2;
	int tesannwyn_called = 0;
	int nquads=0;
	int quadtreelevels = 1;
	llPointList    *points = NULL;
	llTriangleList *triangles = NULL;

	INFOHEADER infoheader;

	//******************
    //open the batch
	//******************
	if (batchname) {
		if (!batch->Open(batchname,"[tes4ll]")) DumpExit();
		batch->ReadCache();
	} else {
		batch->ReadStdin("[tes4ll]");
	}
	mesg->Dump();
		
	int gen_npoints=0;    
	int com = 0;
	std::vector<llAlg*> alg_list;
	alg_list.resize(100);
	int alg_counter=0;
	int triangulation=0;

	llMap *der = NULL; //dervatives

	batch->install_dir="";

	if (!utils->IsEnabled("_gamedir")) {
		if( RegOpenKeyEx(    HKEY_LOCAL_MACHINE, 
			"SOFTWARE\\Bethesda Softworks\\Oblivion",0, 
			KEY_QUERY_VALUE, &keyHandle) == ERROR_SUCCESS) {
			size1=1023;
			RegQueryValueEx( keyHandle, "Installed Path", NULL, &Type, 
				(LPBYTE)rgValue,&size1);
			char *oblivion_path = new char[strlen(rgValue)+2];
			strcpy_s(oblivion_path,strlen(rgValue)+1,rgValue);
			mesg->WriteNextLine(LOG_INFO,"Game path is: %s",oblivion_path);
			utils->SetValue("_gamedir",oblivion_path);
		} else {
			mesg->WriteNextLine(LOG_WARNING,"Game not installed, I will use the working directory.");
			utils->SetValue("_gamedir",".");
			//DumpExit();
		}
		RegCloseKey(keyHandle);
	} else {
		mesg->WriteNextLine(LOG_INFO,"Game path is: %s",utils->GetValue("_gamedir"));
	}

	mesg->Dump();

	mesg->WriteNextLine(LOG_INFO,"****** Go into batch mode ******");

	float minab=256;

	__int64 time_statistics[COM_MAX_CMD];
	int time_statistics_cmd[COM_MAX_CMD];
	char *time_statistics_cmdname[COM_MAX_CMD];
	unsigned int time_statistics_pointer = 0;
	
	//******************
	//batch loop
	//******************

	while ((com = batch->GetCommand())>-2) {
		//cout << com << endl;
		FILETIME idleTime;
		FILETIME kernelTime;
		FILETIME userTime;
		BOOL res = GetSystemTimes( &idleTime, &kernelTime, &userTime );

		mesg->Dump();

		try {

		if (com == COM_PARSEMODLIST) {


			if (!list_string) {
				char oblivion_app_path[1024];
				if( RegOpenKeyEx(    HKEY_CURRENT_USER, 
					"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders",0, 
					KEY_QUERY_VALUE, &keyHandle) == ERROR_SUCCESS) {
						size1=1023;
						strcpy_s(rgValue,1024,"\0");
						RegQueryValueEx( keyHandle, "Local Appdata", NULL, &Type, 
							(LPBYTE)rgValue,&size1); //win7
						if (strlen(rgValue) == 0) {
							strcpy_s(rgValue,1024,"\0");
							RegQueryValueEx( keyHandle, "Appdata", NULL, &Type, 
								(LPBYTE)rgValue,&size1); //win XP
						}
						if (strlen(rgValue) == 0) {
							mesg->WriteNextLine(LOG_FATAL,"Could not get Appdata path!");
							DumpExit();
						}
						strcpy_s(oblivion_app_path,1024,rgValue);
						mesg->WriteNextLine(LOG_INFO,"Appdata path is: %s",oblivion_app_path);
				}     
				else {
					mesg->WriteNextLine(LOG_FATAL,"Could not get Appdata path!");
					DumpExit();
				}
				RegCloseKey(keyHandle);
				char listname[2000];
				sprintf_s(listname,2000,"%s\\Oblivion\\plugins.txt\0",oblivion_app_path);

				if (fopen_s(&fesplist,listname,"r")) {
					mesg->WriteNextLine(LOG_FATAL,"Unable to open plugin file \"%s\"\n",listname);
					DumpExit();
				}
				while (fgets(esp_list[num_esp],1000,fesplist)) {
					if (esp_list[num_esp][0] != '#' && strlen(esp_list[num_esp])>5) {
						//remove the trailing \n
						if (num_esp==256) {
							mesg->WriteNextLine(LOG_FATAL,"Too many plugins\n");
							DumpExit();
						}
						esp_list[num_esp][strlen(esp_list[num_esp])-1] = '\0';

						if (strstr(esp_list[num_esp],",") > 0) {
							mesg->WriteNextLine(LOG_WARNING,"The esp '%s' contains an illegal character - skipped",
								esp_list[num_esp]);
						} else {
							//cout << esp_list[num_esp];
							if (num_esp<256) num_esp++;
							esp_list[num_esp] = new char[1000];
						}
					}
				}

				mesg->WriteNextLine(LOG_INFO,"%i plugins will be used",num_esp);

				for (int i=0; i<num_esp;i++) {
					//open the esp
					WIN32_FILE_ATTRIBUTE_DATA fAt = {0};
					char tmpName2[2000];
					sprintf_s(tmpName2,2000, "%s", esp_list[i]); 
					wchar_t tmpName[2000]; 
					swprintf(tmpName, 2000,L"%s", tmpName2); 
					if (!GetFileAttributesEx(tmpName2,GetFileExInfoStandard,&fAt)) {
						mesg->WriteNextLine(LOG_FATAL,"The esp '%s' was not found",esp_list[i]);
						//cout << GetLastError() << endl;
						DumpExit();
					}
					FILETIME time = fAt.ftLastWriteTime;

					esp_list_sorted[num_esp_sorted]=esp_list[i];
					time_list_sorted[num_esp_sorted]=time;
					num_esp_sorted++;

					for (int j=num_esp_sorted-1;j>0;j--) {  //quicksort
						if (CompareFileTime(&time_list_sorted[j-1],&time_list_sorted[j])>0) {
							FILETIME ttmp = time_list_sorted[j-1];
							char * tmp = esp_list_sorted[j-1];
							time_list_sorted[j-1]=time_list_sorted[j];
							esp_list_sorted[j-1]=esp_list_sorted[j];
							time_list_sorted[j]=ttmp;
							esp_list_sorted[j]=tmp;
						}
					}
				}

				for (int j=0;j<num_esp_sorted;j++) {
					char * my_flag_list=new char[strlen(esp_list_sorted[j])+2];
					strcpy_s(my_flag_list,strlen(esp_list_sorted[j])+1,esp_list_sorted[j]);
					for (unsigned int jj=0;jj<strlen(my_flag_list);jj++) {
						if (*(my_flag_list+jj) == ' ') *(my_flag_list+jj)='_';
					}
					mesg->WriteNextLine(LOG_INFO,"Flag: %s",my_flag_list);
					utils->AddFlag(my_flag_list);
				}
			} else { //list mod option -l provided
				char *ptr;          
				char *saveptr1 = NULL;
				char *list_string2 = new char[strlen(list_string)+2];
				strcpy_s(list_string2,strlen(list_string)+1,list_string);
				ptr = strtok_int(list_string2, ',', &saveptr1);
				while(ptr != NULL) {
					char *flag_list=new char[strlen(ptr)+2];
					strcpy_s(flag_list,strlen(ptr)+1,ptr);
					for (unsigned int j=0;j<strlen(flag_list);j++) {
						if (*(flag_list+j) == ' ') *(flag_list+j)='_';
					}
					ptr = strtok_int(NULL, ',', &saveptr1);
					mesg->WriteNextLine(LOG_INFO,"Flag: %s",flag_list);
					utils->AddFlag(flag_list);
				}
			}
		}

		if (com == COM_CALLTESANNWYN) {
			char all[256*1000];
			if (!list_string) {
				sprintf_s(all,256*1000,"TESAnnwyn.exe -c -p 2 -b 32 -w %s \"%s",utils->GetValue("_worldspace"),esp_list_sorted[0]);
				for (int i=1;i<num_esp_sorted;i++) sprintf_s(all,256*1000,"%s,%s",all,esp_list_sorted[i]);
				sprintf_s(all,256*1000,"%s\"\n",all);
			} else {
				sprintf_s(all,256*1000,"TESAnnwyn.exe -c -p 2 -b 32 -w \"%s\" \"%s\"",utils->GetValue("_worldspace"),list_string);				
			}

			mesg->WriteNextLine(LOG_INFO,"Call Tesannwyn with the following command:\n%s",all);
			mesg->Dump();

			//FILE *tes = _popen("TESAnnwyn.exe -c -p 2 -b 32 -w Tamriel Oblivion.esm","rt");
			FILE *tes = _popen(all,"rt");
			bmpfilename = "tesannwyn.bmp";
			char c; 
			int seekmode=0;
			char testst[1000], testst2[1000];

			if (tes==NULL) {
				mesg->WriteNextLine(LOG_FATAL,"Error calling Tesannwyn");
				DumpExit();
			} else {
				do {
					//cout << seekmode;
					c = fgetc (tes);
					cout << c;
					testst[19]=c;
					if (seekmode>0) {
						testst2[seekmode-1]=c;
						seekmode++;
						if (c == ')') {
							testst2[seekmode-1]='\0';
							seekmode=-1;

						}
					}
					for (int j=0;j<19;j++) testst[j]=testst[j+1];
					if (strncmp (testst,"corresponds to cell",19) ==0) {
						//cout << "Found" << endl;
						seekmode=1;
					}
				} while (c != EOF);
				fclose (tes);
				if (seekmode!=-1) {
					mesg->WriteNextLine(LOG_FATAL,"Tesannwyn failed");
					DumpExit();
				} else {
					sscanf_s(testst2," (%i, %i)",&x_cell,&y_cell);
					mesg->WriteNextLine(LOG_INFO,"****** Tesannwyn finished ******");
					mesg->WriteNextLine(LOG_INFO,"x corner: %i",x_cell);
					mesg->WriteNextLine(LOG_INFO,"y corner: %i",y_cell);
				}
			}
			tesannwyn_called = 1;
		} //end COM_CALLTESANNWYN

		if (com == COM_CALLTES4QLOD) {
			char all[256*1000];
			char int_list_string[256*1000];
			if (!list_string) {
				sprintf_s(int_list_string,256*1000,"%s",esp_list_sorted[0]);
				for (int i=1;i<num_esp_sorted;i++) sprintf_s(int_list_string,256*1000,"%s,%s",int_list_string,esp_list_sorted[i]);
			} else {
				sprintf_s(int_list_string,256*1000,"%s",list_string);				
			}
			sprintf_s(all,256*1000,"tes4qlod.exe %s \"%s\" \"%s\" ",batch->tes4qlod_options,utils->GetValue("_worldspace"),int_list_string);	
			mesg->WriteNextLine(LOG_INFO,"Call build-in TES4qLOD with the following command:\n%s",all);
			mesg->Dump();
			if (strlen(batch->dds_tool) > 1)
				DDS_CONVERTOR = batch->dds_tool;
			if (batch->tes4qlod_silent) verbosity = 0;
			else verbosity = 1;
			calltes4qlodwrapper(all, batch, utils);
		} //end COM_CALLQLOD


		if (com == COM_READBMP) {
			mesg->WriteNextLine(LOG_COMMAND,"%s: reading the bitmap from file %s",COM_READBMP_CMD,bmpfilename);
			mesg->Dump();

			if (!tesannwyn_called && calltesanwynn) {
				mesg->WriteNextLine(LOG_FATAL,"Before reading the bitmap, TESAnnwyn must be called.");
				DumpExit();
			}
			//******************
			//open the bmp and fill the heightmap
			//******************
			
			if (fopen_s(&fptr,bmpfilename,"rb")) {
				mesg->WriteNextLine(LOG_FATAL,"Unable to open BMP file \"%s\"",bmpfilename);
				DumpExit();
			}

			HEADER header;
			
			//COLOURINDEX colourindex[256];

			/* Read and check the header */
			ReadUShort(fptr,&header.type,0);
			//fprintf(stderr,"ID is: %d, should be %d\n",header.type,'M'*256+'B');
			ReadUInt(fptr,&header.size,0);
			mesg->WriteNextLine(LOG_INFO,"File size is %d bytes",header.size);
			ReadUShort(fptr,&header.reserved1,0);
			ReadUShort(fptr,&header.reserved2,0);
			ReadUInt(fptr,&header.offset,0);
			//fprintf(stderr,"Offset to image data is %d bytes\n",header.offset);

			/* Read and check the information header */
			if (fread(&infoheader,sizeof(INFOHEADER),1,fptr) != 1) {
				mesg->WriteNextLine(LOG_FATAL,"Failed to read BMP info header");
				DumpExit();
			}
			mesg->WriteNextLine(LOG_INFO,"Image size = %d x %d",infoheader.width,infoheader.height);
			mesg->WriteNextLine(LOG_INFO,"Bits per pixel is %d",infoheader.bits);

			if (infoheader.bits != 32) {
				mesg->WriteNextLine(LOG_FATAL,"Not a 32-bit file");
				DumpExit();
			}
			int max = 0, min=0;
			if (heightmap) {
				//fprintf(stderr,"[Fatal] Called ReadHeightMap 2 times\n");
				//exit(-1);
				mesg->WriteNextLine(LOG_INFO,"Delete old heightmap");
				if (der && der!=heightmap) delete der;
				delete heightmap;
				heightmap=NULL;
				delete quads;
				delete points;
				delete triangles;
				
			}
			heightmap = new llMap(infoheader.width*batch->npoints, infoheader.height*batch->npoints);
			heightmap->SetScaling(batch->npoints);

			//caluclate coord system
			x1 = x_cell*int(batch->cellsize_x);
			y1 = y_cell*int(batch->cellsize_y);
			x2 = x1 + infoheader.width * 128;
			y2 = y1 + infoheader.height * 128;
			batch->x1 = x_cell*int(batch->cellsize_x);
			batch->y1 = y_cell*int(batch->cellsize_y);
			batch->x2 = batch->x1 + infoheader.width * 128;
			batch->y2 = batch->y1 + infoheader.height * 128;
			//Quadlist
			quadx1=int(floor((float(batch->x1)/batch->quadsize_x)/batch->cellsize_x));
			quady1=int(floor((float(batch->y1)/batch->quadsize_y)/batch->cellsize_y));
			quadx2=int(floor((float(batch->x2)/batch->quadsize_x)/batch->cellsize_x));
			quady2=int(floor((float(batch->y2)/batch->quadsize_y)/batch->cellsize_y));

			nquads=(quadx2-quadx1+1)*(quady2-quady1+1);
			mesg->WriteNextLine(LOG_INFO,"The map covers %i quads",nquads);
			quads= new llQuadList(mesg, nquads);
			for (int ix=quadx1;ix<=quadx2;ix++) {
				for (int iy=quady1;iy<=quady2;iy++) {
					quads->AddQuad(ix,iy,float(ix)*batch->quadsize_x*batch->cellsize_x,
						float(iy)*batch->quadsize_y*batch->cellsize_y,
						float(ix+1)*batch->quadsize_x*batch->cellsize_x,
						float(iy+1)*batch->quadsize_y*batch->cellsize_y);
				}
			}
			quads->SubQuadLevels(batch->quadtreelevels - 1);

			heightmap->SetCoordSystem(x1,y1,x2,y2);

			batch->x00 = (float)batch->x1;
			batch->x11 = (float)batch->x2;
			batch->y00 = (float)batch->y1;
			batch->y11 = (float)batch->y2;
			batch->gridx = batch->cellsize_x;
			batch->gridy = batch->cellsize_y;

			/* Read the image */
			for (int bmp_y=0;bmp_y<infoheader.height;bmp_y++) {
				for (int bmp_x=0;bmp_x<infoheader.width;bmp_x++) {
					int val;
					ReadInt(fptr,&val,0);

					heightmap->SetElement(bmp_x*batch->npoints,bmp_y*batch->npoints,float(val));				

					if (batch->npoints > 1) {
						float x00=heightmap->GetElement(bmp_x*batch->npoints-batch->npoints,bmp_y*batch->npoints-batch->npoints);
						float x10=heightmap->GetElement(bmp_x*batch->npoints,bmp_y*batch->npoints-batch->npoints);
						float x01=heightmap->GetElement(bmp_x*batch->npoints-batch->npoints,bmp_y*batch->npoints);
						//cout << "int" << endl;
						for (int x1=bmp_x*batch->npoints-batch->npoints;x1<=bmp_x*batch->npoints;x1++) {
							for (int y1=bmp_y*batch->npoints-batch->npoints;y1<=bmp_y*batch->npoints;y1++) {
								float frac_x = float(x1-(bmp_x*batch->npoints-batch->npoints))/float(batch->npoints);
								float frac_y = float(y1-(bmp_y*batch->npoints-batch->npoints))/float(batch->npoints);
								
								float myval1=(x00 + frac_x*(x10-x00));
								float myval2=(x01 + frac_x*(val-x01));
								float myval=myval1 + frac_y*(myval2-myval1);
								heightmap->SetElement(x1,y1,myval);			
							}
						}
					}

					if (val > max) max=val;
					if (val < min) min=val;
				}
			}
			mesg->WriteNextLine(LOG_INFO,"Lowest point: %i, highest point: %i",min*8,max*8);

			points    = new llPointList(npoints+4,quads);
			polygons  = new llPolygonList(mesg,points,heightmap);
			triangles = new llTriangleList(npoints, points);
			der = heightmap;
		} //end readbmp


		if (com == COM_GENERATEHEIGHTMAP) {
#ifdef USE_TES4QLOD

			//******************
			//Generate heightmap from the esp list
			//******************
			char all[256*1000];

			mesg->WriteNextLine(LOG_COMMAND,"%s: generate heightmap from esp/esm list",COM_GENERATEHEIGHTMAP_CMD);
			mesg->Dump();

			int max = 0, min=0;
			if (heightmap) {
				mesg->WriteNextLine(LOG_INFO,"Delete old heightmap");
				if (der && der!=heightmap) delete der;
				der=NULL;
				delete heightmap;
				heightmap=NULL;
				delete quads;
				delete points;
				delete triangles;
			}

			char int_list_string[256*1000];
			if (!list_string) {
				sprintf_s(int_list_string,256*1000,"%s",esp_list_sorted[0]);
				for (int i=1;i<num_esp_sorted;i++) sprintf_s(int_list_string,256*1000,"%s,%s",int_list_string,esp_list_sorted[i]);
			} else {
				sprintf_s(int_list_string,256*1000,"%s",list_string);				
			}

			verbosity = 0; //disable tes4qlod blabla
			if (!batch->quick) {
				sprintf_s(all,256*1000,"tes4qlod.exe -x \"%s\" \"%s\" ",utils->GetValue("_worldspace"),int_list_string);	
				mesg->WriteNextLine(LOG_INFO,"Call build-in TES4qLOD with the following command: '%s'",all);
				mesg->Dump();
				
				calltes4qlodwrapper(all, batch, utils);
			}
			//now I know the dimensions
			if (batch->minheight>-999999.f)
				heightmap = new llMap((max_x - min_x + 1)*32*batch->npoints, (max_y - min_y + 1)*32*batch->npoints, 0, (batch->minheight)/8.f);
			else
				heightmap = new llMap((max_x - min_x + 1)*32*batch->npoints, (max_y - min_y + 1)*32*batch->npoints, 0, 0);
			npoints = batch->npoints;
			heightmap->SetScaling(batch->npoints);
			x_cell = min_x;
			y_cell = min_y;
			mesg->WriteNextLine(LOG_INFO,"x corner: %i",x_cell);
			mesg->WriteNextLine(LOG_INFO,"y corner: %i",y_cell);

			//caluclate coord system
			x1 = x_cell*int(batch->cellsize_x);
			y1 = y_cell*int(batch->cellsize_y);
			x2 = x1 + (max_x - min_x + 1) * 32 * 128;
			y2 = y1 + (max_y - min_y + 1) * 32 * 128;
			batch->x1 = x_cell*int(batch->cellsize_x);
			batch->y1 = y_cell*int(batch->cellsize_y);
			batch->x2 = batch->x1 + (max_x - min_x + 1) * 32 * 128;
			batch->y2 = batch->y1 + (max_y - min_y + 1) * 32 * 128;
			//Quadlist
			quadx1=int(floor((float(batch->x1)/batch->quadsize_x)/batch->cellsize_x));
			quady1=int(floor((float(batch->y1)/batch->quadsize_y)/batch->cellsize_y));
			quadx2=int(floor((float(batch->x2)/batch->quadsize_x)/batch->cellsize_x));
			quady2=int(floor((float(batch->y2)/batch->quadsize_y)/batch->cellsize_y));


			nquads=(quadx2-quadx1+1)*(quady2-quady1+1);
			mesg->WriteNextLine(LOG_INFO,"The map covers %i quads",nquads);
			mesg->Dump();
			quads= new llQuadList(mesg, nquads);
			//quads= new llquadlist(1);
		
			for (int ix=quadx1;ix<=quadx2;ix++) {
				for (int iy=quady1;iy<=quady2;iy++) {
					quads->AddQuad(ix,iy,float(ix)*batch->quadsize_x*batch->cellsize_x,
						float(iy)*batch->quadsize_y*batch->cellsize_y,
						float(ix+1)*batch->quadsize_x*batch->cellsize_x,
						float(iy+1)*batch->quadsize_y*batch->cellsize_y);
				}
			}
			quads->SubQuadLevels(batch->quadtreelevels - 1);


			heightmap->SetCoordSystem(x1,y1,x2,y2);

			batch->x00 = (float)batch->x1;
			batch->x11 = (float)batch->x2;
			batch->y00 = (float)batch->y1;
			batch->y11 = (float)batch->y2;
			batch->gridx = batch->cellsize_x;
			batch->gridy = batch->cellsize_y;

			opt_read_heightmap = 1;
			sprintf_s(all,256*1000,"tes4qlod.exe -x \"%s\" \"%s\" ",utils->GetValue("_worldspace"),int_list_string);	
			mesg->WriteNextLine(LOG_INFO,"I have generated the heightmap and call build-in TES4qLOD again for filling the heights");
			mesg->Dump();
			calltes4qlodwrapper(all, batch, utils);
			verbosity = 1;
			opt_read_heightmap = 0;


			/* Evaluate the heightmap */
#if 1
			for (int bmp_y=0;bmp_y<(max_y - min_y + 1)*32;bmp_y++) {
				for (int bmp_x=0;bmp_x<(max_x - min_x + 1)*32;bmp_x++) {
					int val = int(heightmap->GetElement(bmp_x*batch->npoints,bmp_y*batch->npoints));	
					if (batch->minheight>-999999.f && val < int(batch->minheight)/8)
						heightmap->SetElement(bmp_x*batch->npoints,bmp_y*batch->npoints,batch->minheight/8.f);
					
					if (batch->npoints > 1) {
						float x00=heightmap->GetElement(bmp_x*batch->npoints-batch->npoints,bmp_y*batch->npoints-batch->npoints);
						float x10=heightmap->GetElement(bmp_x*batch->npoints,bmp_y*batch->npoints-batch->npoints);
						float x01=heightmap->GetElement(bmp_x*batch->npoints-batch->npoints,bmp_y*batch->npoints);

						for (int x1=bmp_x*batch->npoints-batch->npoints;x1<=bmp_x*batch->npoints;x1++) {
							for (int y1=bmp_y*batch->npoints-batch->npoints;y1<=bmp_y*batch->npoints;y1++) {
								float frac_x = float(x1-(bmp_x*batch->npoints-batch->npoints))/float(batch->npoints);
								float frac_y = float(y1-(bmp_y*batch->npoints-batch->npoints))/float(batch->npoints);
								
								float myval1=(x00 + frac_x*(x10-x00));
								float myval2=(x01 + frac_x*(val-x01));
								float myval=myval1 + frac_y*(myval2-myval1);
								heightmap->SetElement(x1,y1,myval);			
							}
						}
					}

					if (val > max) max=val;
					if (val < min) min=val;
				}
			}
#endif
			mesg->WriteNextLine(LOG_INFO,"Lowest point: %i, highest point: %i",min*8,max*8);

			points    = new llPointList(npoints+4, quads);
			polygons  = new llPolygonList(mesg, points, heightmap);
			triangles = new llTriangleList(npoints, points);
			der = heightmap;
#endif
		} //end readbmp


		if (com == COM_EXIT) {
			mesg->WriteNextLine(LOG_COMMAND,"%s: Bye...", COM_EXIT_CMD);
			DumpExit();
		}

		if (com == COM_SETPATH) {
			mesg->WriteNextLine(LOG_COMMAND,"%s: change directory path to %s", COM_SETPATH_CMD, batch->myflagname);
			_chdir(batch->myflagname);
		}

		if (com == COM_SETGRID) {
			mesg->WriteNextLine(LOG_COMMAND,"%s: -x=%.0f -y=%.0f",COM_SETGRID_CMD, batch->gridx, batch->gridy);
			mesg->Dump();

			if (triangulation) {
				mesg->WriteNextLine(LOG_FATAL,"%s called after triangulation.",COM_SETGRID_CMD);
				DumpExit();
			}
			if (!heightmap) {
				mesg->WriteNextLine(LOG_FATAL,"No heightmap present.");
				DumpExit();
			}

			for (float x=floor(batch->x00/batch->gridx)*batch->gridx;x<=(batch->x11+1);x+=batch->gridx) {
				for (float y=floor(batch->y00/batch->gridy)*batch->gridy;y<=(batch->y11+1);y+=batch->gridy) {
					float x1=x,y1=y;
					if (x>=batch->x11) x1=batch->x11-1;
					if (y>=batch->y11) y1=batch->y11-1;

					if (points->GetMinDistance(x1,y1) > minab) {
						points->AddPoint(x1,y1,heightmap->GetZ(x1,y1));	
						gen_npoints++;
					}
				}
			}
		}

		if (com == COM_SETGRIDBORDER) {
			if (batch->zz1 > -999999.f)
				mesg->WriteNextLine(LOG_COMMAND,"%s: -x=%.0f -y=%.0f -min=%.0f", COM_SETGRIDBORDER_CMD, batch->gridx, batch->gridy, batch->zz1);
			else
				mesg->WriteNextLine(LOG_COMMAND,"%s: -x=%.0f -y=%.0f", COM_SETGRIDBORDER_CMD, batch->gridx, batch->gridy);
			mesg->Dump();

			if (triangulation) {
				mesg->WriteNextLine(LOG_FATAL,"%s called after triangulation.", COM_SETGRIDBORDER_CMD);
				DumpExit();
			}
			if (!heightmap) {
				mesg->WriteNextLine(LOG_FATAL,"No heightmap present.");
				DumpExit();
			}
			
			for (float x=floor(batch->x00/batch->gridx)*batch->gridx;x<=(batch->x11+1);x+=batch->gridx) {
				float x1=x;
				if (x>=batch->x11) x1=batch->x11-1;
				if (points->GetMinDistance(x1,batch->y00) > minab && heightmap->GetZ(x1,batch->y00) > batch->zz1) {
					points->AddPoint(x1,batch->y00,heightmap->GetZ(x1,batch->y00));	
					gen_npoints++;
				}
				if (points->GetMinDistance(x1,batch->y11) > minab && heightmap->GetZ(x1,batch->y11) > batch->zz1) {
					points->AddPoint(x1,batch->y11,heightmap->GetZ(x1,batch->y11));	
					gen_npoints++;
				}
			}
			for (float y=floor(batch->y00/batch->gridy)*batch->gridy;y<=(batch->y11+1);y+=batch->gridy) {
				float y1=y;
				if (y>=batch->y11) y1=batch->y11-1;
				if (points->GetMinDistance(batch->x00,y1) > minab && heightmap->GetZ(batch->x00,y1) > batch->zz1) {
					points->AddPoint(batch->x00,y1,heightmap->GetZ(batch->x00,y1));	
					gen_npoints++;
				}
				if (points->GetMinDistance(batch->x11,y1) > minab && heightmap->GetZ(batch->x11,y1) > batch->zz1) {
					points->AddPoint(batch->x11,y1,heightmap->GetZ(batch->y11,y1));	
					gen_npoints++;
				}
			}
		}
		
		if (com == COM_SETHEIGHT) {

			if (batch->usegameunits) batch->zmin /= 8.0f; //convert to heightmap units

			mesg->WriteNextLine(LOG_COMMAND,"%s: -x1=%.0f -y1=%.0f -x1=%.0f -y1=%.0f -z=%.0f", COM_SETHEIGHT_CMD,
				batch->x00, batch->x11, batch->y00, batch->y11, batch->zmin);
			if (!heightmap) {
				mesg->WriteNextLine(LOG_FATAL,"No heightmap present.");
				DumpExit();
			}

			int x1 = (int) (batch->x00 / 128.f);
			int x2 = (int) (batch->x11 / 128.f);
			int y1 = (int) (batch->y00 / 128.f);
			int y2 = (int) (batch->y11 / 128.f);

			for (int x=x1;x<=x2;x+=1) {
				for (int y=y1;y<=y2;y+=1) {
					for (int xx=0;xx<heightmap->GetScaling();xx+=1) {
						for (int yy=0;yy<heightmap->GetScaling();yy+=1) {
							heightmap->SetElement((x - x_cell *32 - 1)*heightmap->GetScaling()+xx, 
								(y - y_cell *32 - 1)*heightmap->GetScaling()+yy, batch->zmin);
						}
					} 
				}
			}
		}

		if (com == COM_BREAKATGRID) {
			mesg->WriteNextLine(LOG_INFO,"%s: -x=%.0f -y=%.0f -max=%.0f -zmin=%.0f", COM_BREAKATGRID_CMD,
				batch->gridx, batch->gridy, batch->max, batch->zmin);
			mesg->Dump();

			if (triangulation) {
				mesg->WriteNextLine(LOG_FATAL,"BreakAtGrid called after triangulation.");
				DumpExit();				
			}
			if (!heightmap) {
				mesg->WriteNextLine(LOG_FATAL,"No heightmap present.");
				DumpExit();
			}
			int gb_points=0;
			
			for (float x=floor(batch->x00/batch->gridx)*batch->gridx;x<=(batch->x11+1)-batch->gridx;x+=batch->gridx) {
				for (float y=floor(batch->y00/batch->gridy)*batch->gridy;y<=(batch->y11+1)-batch->gridy;y+=batch->gridy) {
					
					//List of segments
					float segstart[1000];
					float segend[1000];
					int segaktive[1000];
					int segpointer=0;

					//fill the first segment
					segstart[segpointer]=x;
					segend[segpointer]=x+batch->gridx;
					segaktive[segpointer]=1;
					segpointer++;

					for (int i=0;i<segpointer;i++) {

						//is segment large enough?
						if (segend[i]-segstart[i]>2*minab && segaktive[i]) {
							float mymax=-1;
							float myx=segstart[i]+minab;
							float z=heightmap->GetZ(segstart[i]+minab,y);
							float slope=(heightmap->GetZ(segend[i]-minab,y) -z ) / (segend[i] - segstart[i] - 2*minab);

							for (float x1 = segstart[i]+minab ;x1 < segend[i]-minab; x1++) {
								float walldiff = (z + slope * (x1 - (segstart[i]+minab))) - heightmap->GetZ(x1,y);
								if (walldiff > batch->max && walldiff>mymax && heightmap->GetZ(x1,y)>batch->zmin) {
									mymax=walldiff;
									myx = x1;
								}
							}

							if (mymax>0) {
								//Split segment
								points->AddPoint(myx,y,heightmap->GetZ(myx,y));	
								gen_npoints++;
								gb_points++;
								segaktive[i]=0;
								segstart[segpointer]=segstart[i];
								segend[segpointer]=myx;
								segaktive[segpointer]=1;
								segpointer++;
								segstart[segpointer]=myx;
								segend[segpointer]=segend[i];
								segaktive[segpointer]=1;
								segpointer++;
								i=0;
							} else segaktive[i]=0;

						} else segaktive[i]=0;
					}

					segpointer=0;
					//fill the first segment
					segstart[segpointer]=y;
					segend[segpointer]=y+batch->gridy;
					segaktive[segpointer]=1;
					segpointer++;

					for (int i=0;i<segpointer;i++) {

						//is segment large enough?
						if (segend[i]-segstart[i]>2*minab && segaktive[i]) {
							float mymax=-1;
							float myy=segstart[i]+minab;
							float z=heightmap->GetZ(x,segstart[i]+minab);
							float slope=(heightmap->GetZ(x,segend[i]-minab) -z ) / (segend[i] - segstart[i] - 2*minab);

							for (float y1 = segstart[i]+minab ;y1 < segend[i]-minab; y1++) {
								float walldiff = (z + slope * (y1 - (segstart[i]+minab))) - heightmap->GetZ(x,y1);
								if (walldiff > batch->max && walldiff>mymax && heightmap->GetZ(x,y1)>batch->zmin) {
									mymax=walldiff;
									myy = y1;
								}
							}

							if (mymax>0) {
								//Split segment
								points->AddPoint(x,myy,heightmap->GetZ(x,myy));	
								gen_npoints++;
								gb_points++;
								segaktive[i]=0;
								segstart[segpointer]=segstart[i];
								segend[segpointer]=myy;
								segaktive[segpointer]=1;
								segpointer++;
								segstart[segpointer]=myy;
								segend[segpointer]=segend[i];
								segaktive[segpointer]=1;
								segpointer++;
								i=0;
							} else segaktive[i]=0;

						} else segaktive[i]=0;
					}
#if 0
					if (points->GetMinDistance(x1,y1) > minab) {
						points->AddPoint(x1,y1,heightmap->GetZ(x1,y1));	
						gen_npoints++;
					}
#endif
				}
			}
			mesg->WriteNextLine(LOG_INFO,"%i break vertices set",gb_points);
		}

		if (com == COM_PANORAMA) {
			mesg->WriteNextLine(LOG_COMMAND,"%s: -x=%.0f -y=%.0f", COM_PANORAMA_CMD, batch->gridx, batch->gridy);
			mesg->Dump();

			if (triangulation) {
				mesg->WriteNextLine(LOG_FATAL,"%s called after triangulation.", COM_PANORAMA_CMD);
				DumpExit();
			}
			if (!heightmap) {
				mesg->WriteNextLine(LOG_FATAL,"No heightmap present.");
				DumpExit();
			}
			float minab2 = 1024; //BUGBUG
			int pan_points=0;

			if (!heightmap->IsInMap(batch->gridx,batch->gridy)) {
				mesg->WriteNextLine(LOG_ERROR,"Point not in map");
				goto panorama_end;
			}
			float myz=heightmap->GetZ(batch->gridx,batch->gridy); 
			for (float y=batch->y00;y<batch->y11;y+=minab2) {
				float deg = atan((batch->gridy - y)/(batch->gridx - batch->x00));
				float minab_x = minab*cos(deg);
				float keepout_x = batch->Keepout*cos(deg);
				float minab_y = minab*sin(deg);
				float keepout_y = batch->Keepout*sin(deg);
				float y1=y;
				float minx=0,miny=0,minz=-999999;
				for (float x1=batch->x00;x1< (batch->gridx-keepout_x); x1+=minab_x) {
					float dist=sqrt(pow(batch->gridx-x1,2)  + pow(batch->gridy-y1,2));
					float distz=heightmap->GetZ(x1,y1)-myz;
					if (distz/dist > minz) {
						minz = distz/dist;
						minx=x1;miny=y1;
					}
					y1+=minab_y;
				}
				if (minz>-99998) {
					if (points->GetMinDistance(minx,miny) > minab && points->GetMinDistanceGrid(minx,miny) > minab && heightmap->GetZ(minx,miny)>myz) {
						points->AddPoint(minx,miny,heightmap->GetZ(minx,miny));	
						pan_points++;
					}
				}
			} //for y
			for (float y=batch->y00;y<batch->y11;y+=minab2) {
				float deg = atan((batch->gridy - y)/(batch->gridx - batch->x11));
				float minab_x = minab*cos(deg);
				float keepout_x = batch->Keepout*cos(deg);
				float minab_y = minab*sin(deg);
				float keepout_y = batch->Keepout*sin(deg);
				float y1=y;
				float minx=0,miny=0,minz=-999999;
				for (float x1=batch->x11;x1> (batch->gridx+keepout_x); x1-=minab_x) {
					float dist=sqrt(pow(batch->gridx-x1,2)  + pow(batch->gridy-y1,2));
					float distz=heightmap->GetZ(x1,y1)-myz;
					if (distz/dist > minz) {
						minz = distz/dist;
						minx=x1;miny=y1;
					}
					y1-=minab_y;
				}
				if (minz>-99998) {
					if (points->GetMinDistance(minx,miny) > minab && points->GetMinDistanceGrid(minx,miny) > minab && heightmap->GetZ(minx,miny)>myz) {
						points->AddPoint(minx,miny,heightmap->GetZ(minx,miny));	
						pan_points++;
					}
				}
			} //for y
			for (float x=batch->x00;x<batch->x11;x+=minab2) {
				float deg = atan((batch->gridx - x)/(batch->gridy - batch->y00));
				float minab_y = minab*cos(deg);
				float keepout_y = batch->Keepout*cos(deg);
				float minab_x = minab*sin(deg);
				float keepout_x = batch->Keepout*sin(deg);
				float x1=x;
				float minx=0,miny=0,minz=-999999;
				for (float y1=batch->y00;y1< (batch->gridy-keepout_y); y1+=minab_y) {
					float dist=sqrt(pow(batch->gridy-y1,2)  + pow(batch->gridx-x1,2));
					float distz=heightmap->GetZ(x1,y1)-myz;
					if (distz/dist > minz) {
						minz = distz/dist;
						minx=x1;miny=y1;
					}
					x1+=minab_x;
				}
				if (minz>-99998) {
					if (points->GetMinDistance(minx,miny) > minab && points->GetMinDistanceGrid(minx,miny) > minab && heightmap->GetZ(minx,miny)>myz) {
						points->AddPoint(minx,miny,heightmap->GetZ(minx,miny));	
						pan_points++;
					}
				} 			
			} //for x
			for (float x=batch->x00;x<batch->x11;x+=minab2) {
				float deg = atan((batch->gridx - x)/(batch->gridy - batch->y11)); //!
				float minab_y = minab*cos(deg);
				float keepout_y = batch->Keepout*cos(deg);
				float minab_x = minab*sin(deg);
				float keepout_x = batch->Keepout*sin(deg);
				float x1=x;
				float minx=0,miny=0,minz=-999999;
				for (float y1=batch->y11;y1> (batch->gridy+keepout_y); y1-=minab_y) { //!
					float dist=sqrt(pow(batch->gridy-y1,2)  + pow(batch->gridx-x1,2));
					float distz=heightmap->GetZ(x1,y1)-myz;
					if (distz/dist > minz) {
						minz = distz/dist;
						minx=x1;miny=y1;
					}
					x1-=minab_x;
				}
				if (minz>-99998) {
					if (points->GetMinDistance(minx,miny) > minab && points->GetMinDistanceGrid(minx,miny) > minab && heightmap->GetZ(minx,miny)>myz) {
						points->AddPoint(minx,miny,heightmap->GetZ(minx,miny));	
						pan_points++;
					}
				} 		
			} //for x
			mesg->WriteNextLine(LOG_INFO,"%i panorama vertices set",pan_points);

panorama_end: ;

		}

		if (com == COM_SETSINGLEPOINT) {
			mesg->WriteNextLine(LOG_COMMAND,"%s: -x=%.0f -y=%.0f", COM_SETSINGLEPOINT_CMD, batch->gridx, batch->gridy);
			mesg->Dump();

			if (triangulation) {
				mesg->WriteNextLine(LOG_FATAL,"%s called after triangulation.", COM_SETSINGLEPOINT_CMD);
				DumpExit();
			}
			if (!heightmap) {
				mesg->WriteNextLine(LOG_FATAL,"No heightmap present.");
				DumpExit();
			}
			
			if (!heightmap->IsInMap(batch->gridx,batch->gridy)) {
				mesg->WriteNextLine(LOG_ERROR,"Point not in map");
			} else {
				points->AddPoint(batch->gridx, batch->gridy, heightmap->GetZ(batch->gridx,batch->gridy));	
			}
		}

		if (com == COM_READFILE) {
			mesg->WriteNextLine(LOG_COMMAND,"%s: %s", COM_READFILE_CMD, batch->datafile);
			mesg->Dump();

			if (triangulation) {
				mesg->WriteNextLine(LOG_FATAL,"%s called after triangulation.", COM_READFILE_CMD);
				DumpExit();
			}
			if (!heightmap) {
				mesg->WriteNextLine(LOG_FATAL,"No heightmap present.");
				DumpExit();
			}
			FILE *fptr_data;

			if (fopen_s(&fptr_data,batch->datafile,"r")) {
				mesg->WriteNextLine(LOG_FATAL,"Unable to open Data file \"%s\"",batch->datafile);
				DumpExit();
			}

			char line[1000];
			char *linex = line;
			size_t size=1000;
			int lp=0;
			while (fgets(line,1000,fptr_data)) {
				utils->StripSpaces(&linex);
				float x,y;
				if ((strlen(linex)>2) && (linex[0]!=';') && (linex[0]!='#') && (linex[0]!='[')) {
					if (sscanf_s(line,"%f %f",&x,&y) == 2) {
						if (!heightmap->IsInMap(x,y)) {
							mesg->WriteNextLine(LOG_ERROR,"Point (%.0f,%.0f) not in map",x,y);
						} else {
							points->AddPoint(x,y,heightmap->GetZ(x,y));	
							lp++;
						}
					} else 
						mesg->WriteNextLine(LOG_ERROR,"Syntax error in data file \"%s\" in the line: %i",batch->datafile,line);
				}
			}
			mesg->WriteNextLine(LOG_INFO,"%s: %i vertex points added from data file %s", COM_READFILE_CMD, lp, batch->datafile);
		}

		if (com == COM_SETOPTION) {
			if (batch->quadtreelevels > 1) {
				quadtreelevels = batch->quadtreelevels;
				batch->quadtreelevels = 1;
				mesg->WriteNextLine(LOG_INFO,"SetOption -quadtreelevels=%s", quadtreelevels);
				quads->SubQuadLevels(quadtreelevels - 1);
			}
			if (batch->mindistance>0) {
				mesg->WriteNextLine(LOG_INFO,"SetOption -mindistance=%i", batch->mindistance);
				minab = float(batch->mindistance);
			}
			if (batch->nquadmax>0) {
				mesg->WriteNextLine(LOG_INFO,"SetOption -nquadmax=%i", batch->nquadmax);
				quads->SetMaxPoints(batch->nquadmax);
			}
			opt_size_x = batch->size_x;
			opt_size_y = batch->size_y;
			opt_center = batch->center;
		}

		if (com == COM_FILTER) {
			mesg->WriteNextLine(LOG_COMMAND,"%s: -n=%i", COM_FILTER_CMD, batch->npoints);
			mesg->Dump();
			
			if (!heightmap) {
				mesg->WriteNextLine(LOG_FATAL,"No heightmap present.");
				DumpExit();
			}
			
			der = heightmap->Filter(batch->npoints, int(batch->overwrite), batch);
			der->MakeDerivative(batch->use16bit);
			mesg->WriteNextLine(LOG_COMMAND,"%s: done", COM_FILTER_CMD);
		}

		if (com == COM_BREAKLINE) {
			if (triangulation) {
				mesg->WriteNextLine(LOG_COMMAND,"%s, %i new triangles added", COM_BREAKLINE_CMD, triangles->DivideAtZ(batch->z,minab,heightmap));
			} else {
				mesg->WriteNextLine(LOG_COMMAND,"%s: -x=%.0f -y=%.0f -z=%.0f -offsetx=%.0f -offsety=%.0f", 
					COM_BREAKLINE_CMD, batch->gridx, batch->gridy, batch->z, batch->offsetx, batch->offsety);
			if (batch->findmin) mesg->AddToLine(" -findmin");
			if (batch->findmax) mesg->AddToLine(" -findmax");
			if (batch->linear) mesg->AddToLine(" -linear");
			if (batch->onlyintracell) mesg->AddToLine(" -onlyintracell");
			mesg->Dump();

			if (!heightmap) {
				mesg->WriteNextLine(LOG_FATAL,"No heightmap present.");
				DumpExit();
			}
		    int con_points=0;

			int linear=batch->linear, findmin=batch->findmin, findmax=batch->findmax; 
			
			for (float y=batch->y00+batch->offsety+batch->gridy;y<=(batch->y11-1);y+=batch->gridy) {
				int lastflag=0;
				float minx=0;
				float minz=999999;
				float maxx=0;
				float maxz=-999999;
				for (float x=batch->x00+batch->offsetx+batch->gridx;x<=(batch->x11-1);x+=batch->gridx) {
					if (lastflag==-1 && findmin) {
						for (float x1=x-batch->gridx; x1<=x;x1++) {
							if (heightmap->GetZ(x1,y)<minz) {
								minz=heightmap->GetZ(x1,y);minx=x1;
								if (batch->onlyintracell && int(x1/batch->cellsize_x)*int(batch->cellsize_x)==int(x1)) {
									lastflag=0;
									minz=-999999;
								}
							}
						}
					}
					if (lastflag==1 && findmax) {
						for (float x1=x-batch->gridx; x1<=x;x1++) {
							if (heightmap->GetZ(x1,y)>maxz) {
								maxz=heightmap->GetZ(x1,y);maxx=x1;
								if (batch->onlyintracell && int(x1/batch->cellsize_x)*int(batch->cellsize_x)==int(x1)) {
									lastflag=0;
									minz=-999999;
								}
							}
						}
					}
					

					if (((heightmap->GetZ(x,y) > batch->z ) && (heightmap->GetZ(x-batch->gridx,y)<batch->z))
						|| ((heightmap->GetZ(x,y) < batch->z ) && (heightmap->GetZ(x-batch->gridx,y)>batch->z))) {
							//breakline crossing
							float myx=x-batch->gridx*(heightmap->GetZ(x,y)-batch->z)/
								(heightmap->GetZ(x,y)
								- heightmap->GetZ(x-batch->gridx,y));
							if (!linear) {
								myx=0;
								float numx=0;
								for (float x1=x-batch->gridx; x1<=x;x1++) {
									if ((heightmap->GetZ(x1,y) < batch->z && heightmap->GetZ(x1-minab,y) >= batch->z)
										|| (heightmap->GetZ(x1,y) >batch->z && heightmap->GetZ(x1-minab,y) <= batch->z)) {myx+=x1;numx++;}
								}
								if (numx) {
									myx /= numx;
								} else {
									mesg->WriteNextLine(LOG_WARNING,"ContourLine: contour not found for x");
								}
							}
							int flag=1;
							if (heightmap->GetZ(x,y) < heightmap->GetZ(x-batch->gridx,y)) flag=-1;
							if (lastflag==-1 && flag==1 && minz<99998) { 
								if (points->GetMinDistance(minx,y) > minab) { //  && 
									if (points->GetMinDistanceGrid(minx,y,1) > minab) {
										//points->AddPoint(minx,y,heightmap->GetZ(minx,y));	
									    points->AddPoint(minx,y,batch->z);con_points++;	
										minz=99999;
									}
								} else points->SetFlag(points->GetClosestPoint(minx,y),flag);
							}
							if (lastflag==1 && flag==-1 && maxz>-99998) { 
								if (points->GetMinDistance(maxx,y) > minab) { //  && 
									if (points->GetMinDistanceGrid(maxx,y,1) > minab) {
										//points->AddPoint(minx,y,heightmap->GetZ(minx,y));	
									    points->AddPoint(maxx,y,batch->z);con_points++;	
										maxz=-99999;
									}
								} else points->SetFlag(points->GetClosestPoint(maxx,y),flag);
							}
							//float minab_grid=-1;
							float minab_grid=minab;
							if (points->GetMinDistance(myx,y) > minab && points->GetMinDistanceGrid(myx,y,1) > minab_grid) {
								int p = points->AddPoint(myx,y,heightmap->GetZ(myx,y));	con_points++;
								points->SetFlag(p,flag);
								gen_npoints++;
								if (batch->setmin && heightmap->GetZ(x-batch->gridx,y)<batch->z && points->GetMinDistance(x-batch->gridx,y) > minab)
								{points->AddPoint(x-batch->gridx,y,heightmap->GetZ(x-batch->gridx,y)); gen_npoints++;con_points++;}
								if (batch->setmin && heightmap->GetZ(x,y) < batch->z && points->GetMinDistance(x,y) > minab)
								{points->AddPoint(x,y,heightmap->GetZ(x,y)); gen_npoints++;con_points++;}
								if (batch->setmax && heightmap->GetZ(x-batch->gridx,y)>batch->z && points->GetMinDistance(x-batch->gridx,y) > minab)
								{points->AddPoint(x-batch->gridx,y,heightmap->GetZ(x-batch->gridx,y)); gen_npoints++;con_points++;}
								if (batch->setmax && heightmap->GetZ(x,y) > batch->z && points->GetMinDistance(x,y) > minab)
								{points->AddPoint(x,y,heightmap->GetZ(x,y)); gen_npoints++;con_points++;}

							}
							lastflag=flag;
					}
				}

			}
			for (float x=batch->x00+batch->offsetx+batch->gridx;x<=(batch->x11-1);x+=batch->gridx) {
				int lastflag=0;
				float miny=0;
				float minz=999999;
				float maxy=0;
				float maxz=-999999;
				for (float y=batch->y00+batch->offsety+batch->gridy;y<=(batch->y11-1);y+=batch->gridy) {

					if (lastflag==-1 && findmin) {
						for (float y1=y-batch->gridy; y1<=y;y1++) {
							if (heightmap->GetZ(x,y1)<minz) {
								minz=heightmap->GetZ(x,y1);miny=y1;
								if (batch->onlyintracell && int(y1/batch->cellsize_y)*int(batch->cellsize_y)==int(y1)) {
									lastflag=0;
									minz=-999999;
								}
							}
						}
					}
					if (lastflag==1 && findmax) {
						for (float y1=y-batch->gridy; y1<=y;y1++) {
							if (heightmap->GetZ(x,y1)>maxz) {
								maxz=heightmap->GetZ(x,y1);maxy=y1;
								if (batch->onlyintracell && int(y1/batch->cellsize_y)*int(batch->cellsize_y)==int(y1)) {
									lastflag=0;
									minz=-999999;
								}
							}
						}
					}
					

					if (((heightmap->GetZ(x,y) > batch->z ) && (heightmap->GetZ(x,y-batch->gridy)<batch->z))
						|| ((heightmap->GetZ(x,y) < batch->z ) && (heightmap->GetZ(x,y-batch->gridy)>batch->z))) {
							//breakline crossing
							float myy=y-batch->gridy*(heightmap->GetZ(x,y)-batch->z)/
								(heightmap->GetZ(x,y)
								- heightmap->GetZ(x,y-batch->gridy));
							if (!linear) {
								myy=0;
								float numy=0;
								for (float y1=y-batch->gridy; y1<=y;y1++) {
									if ((heightmap->GetZ(x,y1) < batch->z && heightmap->GetZ(x,y1-minab) >= batch->z)
										|| (heightmap->GetZ(x,y1) >batch->z && heightmap->GetZ(x,y1-minab) <= batch->z)) {myy+=y1;numy++;}
								}
								if (numy) {
									myy /= numy;
								} else {
									mesg->WriteNextLine(LOG_WARNING,"ContourLine: contour not found for y");
								}
							}
							int flag=1;
							if (heightmap->GetZ(x,y) < heightmap->GetZ(x,y-batch->gridy)) flag=-1;
							if (lastflag==-1 && flag==1 && minz<99998) { 
								if (points->GetMinDistance(x,miny) > minab) { 
									if (points->GetMinDistanceGrid(x,miny,2) > minab) {
										//points->AddPoint(x,miny,heightmap->GetZ(x,miny));	
										points->AddPoint(x,miny,batch->z);con_points++;	
										minz=99999;
									}
								} else points->SetFlag(points->GetClosestPoint(x,miny),flag);
							}
							if (lastflag==1 && flag==-1 && maxz>-99998) { 
								if (points->GetMinDistance(x,maxy) > minab) { 
									if (points->GetMinDistanceGrid(x,maxy,2) > minab) {
										//points->AddPoint(x,miny,heightmap->GetZ(x,miny));	
										points->AddPoint(x,maxy,batch->z);con_points++;	
										minz=99999;
									}
								} else points->SetFlag(points->GetClosestPoint(x,maxy),flag);
							}
							//float minab_grid=-1;
							float minab_grid=minab;
							if (points->GetMinDistance(x,myy) > minab && points->GetMinDistanceGrid(x,myy,2) > minab_grid) {
								int p = points->AddPoint(x,myy,heightmap->GetZ(x,myy));	con_points++;
								points->SetFlag(p,flag);
								gen_npoints++;
								if (batch->setmin && heightmap->GetZ(x,y-batch->gridy)<batch->z && points->GetMinDistance(x,y-batch->gridy) > minab)
								{points->AddPoint(x,y-batch->gridy,heightmap->GetZ(x,y-batch->gridy)); gen_npoints++;con_points++;}
								if (batch->setmin && heightmap->GetZ(x,y) < batch->z && points->GetMinDistance(x,y) > minab)
								{points->AddPoint(x,y,heightmap->GetZ(x,y)); gen_npoints++;con_points++;}
								if (batch->setmax && heightmap->GetZ(x,y-batch->gridy)>batch->z && points->GetMinDistance(x,y-batch->gridy) > minab)
								{points->AddPoint(x,y-batch->gridy,heightmap->GetZ(x,y-batch->gridy)); gen_npoints++;con_points++;}
								if (batch->setmax && heightmap->GetZ(x,y) > batch->z && points->GetMinDistance(x,y) > minab)
								{points->AddPoint(x,y,heightmap->GetZ(x,y)); gen_npoints++;con_points++;}

							}
							lastflag=flag;
					}
				}
			}
			mesg->WriteNextLine(LOG_COMMAND,"%s: %i vertices set", COM_BREAKLINE_CMD, con_points);
			}
		}

		if (com == COM_DIVIDEGRID) {
			mesg->WriteNextLine(LOG_COMMAND,"%s: -x=%.0f -y=%.0f", COM_DIVIDEGRID_CMD, batch->gridx, batch->gridy);
			mesg->Dump();

			if (!triangulation) {
				mesg->WriteNextLine(LOG_FATAL,"%s called but no triangulation. Call MakeTriangulation before", COM_DIVIDEGRID_CMD);
				DumpExit();
			}
			if (!heightmap) {
				mesg->WriteNextLine(LOG_FATAL,"No heightmap present.");
				DumpExit();
			}
			
			for (float y=floor(batch->y00/batch->gridy)*batch->gridy;y<=batch->y11;y+=batch->gridy) {
				triangles->DivideAt(false,y,heightmap);    
			}

			for (float x=floor(batch->x00/batch->gridx)*batch->gridx;x<=batch->x11;x+=batch->gridx) {
				triangles->DivideAt(true,x,heightmap);    
			}
			mesg->WriteNextLine(LOG_COMMAND,"%s: done", COM_DIVIDEGRID_CMD);
		}

		if (com == COM_BREAKFLATTRIANGLES) {
			if (batch->Highest < batch->Lowest) {
				mesg->WriteNextLine(LOG_ERROR,"%s: Low must be lower as High", COM_BREAKFLATTRIANGLES_CMD);
			} else {
				mesg->WriteNextLine(LOG_COMMAND,"%s -low=%.0f, -high=%.0f, -z=%.0f", 
					COM_BREAKFLATTRIANGLES_CMD, batch->Lowest, batch->Highest,batch->z);
				mesg->Dump();
				mesg->WriteNextLine(LOG_INFO,"%i triangles splitted",triangles->SplitFlatTriangles(batch->Lowest,batch->Highest,batch->z,heightmap));
			}

			if (!triangulation) {
				mesg->WriteNextLine(LOG_FATAL,"%s called but no triangulation. Call MakeTriangulation before", COM_BREAKFLATTRIANGLES_CMD);
				DumpExit();
			}
			if (!heightmap) {
				mesg->WriteNextLine(LOG_FATAL,"No heightmap present.");
				DumpExit();
			}			
		}

		if (com == COM_REMOVEBROKENTRIANGLES) {
			mesg->WriteNextLine(LOG_COMMAND,"%s", COM_REMOVEBROKENTRIANGLES_CMD);
			mesg->Dump();
			if (!heightmap) {
				mesg->WriteNextLine(LOG_FATAL,"No heightmap present.");
				DumpExit();
			}
			mesg->WriteNextLine(LOG_COMMAND,"%s: %i triangles corrected", 
				COM_REMOVEBROKENTRIANGLES_CMD, triangles->RemoveBrokenTriangles(heightmap));
		}

		if (com == COM_DIVIDEAT) {
			mesg->WriteNextLine(LOG_COMMAND,"%s:", COM_DIVIDEAT_CMD);
			if (batch->gridx>-1000000.) {
				mesg->AddToLine(" -x=",batch->gridx);
			}
			if (batch->gridy>-1000000.) {
				mesg->AddToLine(" -y=",batch->gridy);
			}

			if (!triangulation) {
				mesg->WriteNextLine(LOG_FATAL,"%s called without triangulation. Call MakeTriangulation before", COM_DIVIDEAT_CMD);
				DumpExit();
			}
			if (!heightmap) {
				mesg->WriteNextLine(LOG_FATAL,"No heightmap present.");
				DumpExit();
			}
			if (batch->gridx>-1000000.) {
				triangles->DivideAt(true,batch->gridx,heightmap);    	    
			}
			if (batch->gridy>-1000000.) {
				triangles->DivideAt(false,batch->gridy,heightmap);    	    
			}
			mesg->WriteNextLine(LOG_COMMAND,"%s: done", COM_DIVIDEAT_CMD);
		}

		if (com == COM_DIVIDEBETWEEN) {
			mesg->WriteNextLine(LOG_COMMAND,"%s -x1=%f -y1=%f -x2=%f -y2=%f ", 
				COM_DIVIDEBETWEEN_CMD, batch->xx1, batch->yy1, batch->xx2, batch->yy2);		
			mesg->Dump();	

			if (!triangulation) {
				mesg->WriteNextLine(LOG_FATAL,"%s called without triangulation. Call MakeTriangulation before", COM_DIVIDEBETWEEN_CMD);
				DumpExit();
			}
			if (!heightmap) {
				mesg->WriteNextLine(LOG_FATAL,"No heightmap present.");
				DumpExit();
			}
			triangles->DivideBetween((batch->xx1),(batch->yy1),(batch->xx2),(batch->yy2),heightmap);    	    			
			mesg->WriteNextLine(LOG_COMMAND,"%s: done", COM_DIVIDEBETWEEN_CMD);
		}

		if (com == COM_DIVIDEATPOLGONBORDER) {
			if (batch->polygon_name)
				mesg->WriteNextLine(LOG_COMMAND,"%s -name=%s", COM_DIVIDEATPOLGONBORDER_CMD, batch->polygon_name);			
			else
				mesg->WriteNextLine(LOG_COMMAND,"%s", COM_DIVIDEATPOLGONBORDER_CMD);		
			mesg->Dump();

			if (!triangulation) {
				mesg->WriteNextLine(LOG_FATAL,"%s called without triangulation. Call MakeTriangulation before", 
					COM_DIVIDEATPOLGONBORDER_CMD);
				DumpExit();
			}
			if (!heightmap) {
				mesg->WriteNextLine(LOG_FATAL,"No heightmap present.");
				DumpExit();
			}
			
			llPolygon *mypoly = NULL; 
			if (batch->polygon_name) {
				mypoly = polygons->GetPolygon(batch->polygon_name);
				if (mypoly) {
					for (unsigned int i = 0; i < (unsigned int) mypoly->GetSize()-1;i++) {
						int i1= mypoly->GetPoint(i);
						int i2= mypoly->GetPoint(i+1);
						if (i1>=0 && i2>=0) {
							triangles->DivideBetween(points->GetX(i1),points->GetY(i1),points->GetX(i2),points->GetY(i2),heightmap);    	
						}
					}				
				} else {
					mesg->WriteNextLine(LOG_ERROR,"%s: Polygon not found.", COM_DIVIDEATPOLGONBORDER_CMD);
				}
			} else {
				for (int i=0;i<polygons->GetSize();i++) {
					mypoly = polygons->GetPolygon(i);
					for (unsigned int i = 0; i < (unsigned int) mypoly->GetSize()-1;i++) {
						int i1= mypoly->GetPoint(i);
						int i2= mypoly->GetPoint(i+1);
						if (i1>=0 && i2>=0) {
							triangles->DivideBetween(points->GetX(i1),points->GetY(i1),points->GetX(i2),points->GetY(i2),heightmap);    	
						}
					}	
				}
			}
			mesg->WriteNextLine(LOG_COMMAND,"%s: done", COM_DIVIDEATPOLGONBORDER_CMD);
		}

		if (com == COM_STENCILPOLGON) {
			mesg->WriteNextLine(LOG_COMMAND,"%s (%s)", COM_STENCILPOLGON_CMD, batch->polygon_name);			
			mesg->Dump();

			if (!triangulation) {
				mesg->WriteNextLine(LOG_FATAL,"%s called without triangulation. Call MakeTriangulation before", COM_STENCILPOLGON_CMD);
				DumpExit();
			}
			if (!heightmap) {
				mesg->WriteNextLine(LOG_FATAL,"No heightmap present.");
				DumpExit();
			}
			
			llPolygon *mypoly = polygons->GetPolygon(batch->polygon_name);
			int v = triangles->GetN();
			if (mypoly) {
				//......
				for (unsigned int i = 0; i < (unsigned int) mypoly->GetSize()-1;i++) {
					int i1= mypoly->GetPoint(i);
					int i2= mypoly->GetPoint(i+1);
					if (i1>=0 && i2>=0) {
						triangles->DivideBetween(points->GetX(i1),points->GetY(i1),points->GetX(i2),points->GetY(i2),heightmap);    	
					}
				}
				//........as above
				v = triangles->GetN();
				for (int i=0;i<v;i++) {

					int o1 = triangles->GetPoint1(i);
					int o2 = triangles->GetPoint2(i);
					int o3 = triangles->GetPoint3(i);

					//all 3 points and the center of gravity inside?

					int num = /*mypoly->IsPointInsidePolygon(points->GetX(o1),points->GetY(o1)) +
						mypoly->IsPointInsidePolygon(points->GetX(o2),points->GetY(o2)) +
						mypoly->IsPointInsidePolygon(points->GetX(o3),points->GetY(o3)) +*/
						mypoly->IsPointInsidePolygon((points->GetX(o1)+points->GetX(o2)+points->GetX(o3))/3.f,
						(points->GetY(o1)+points->GetY(o2)+points->GetY(o3))/3.f);

					if (num == 1) {
						triangles->RemoveTriangle(i);
						i--;v--;
					}

				}
				mesg->WriteNextLine(LOG_COMMAND,"%s: done", COM_STENCILPOLGON_CMD);
			} else {
				mesg->WriteNextLine(LOG_ERROR,"%s: Polygon not found.", COM_STENCILPOLGON_CMD);
			}
		}

		if (com == COM_CREATEPOLYGON) {
			mesg->WriteNextLine(LOG_COMMAND,"%s (%s) -x1=%.0f -y1=%.0f -x2=%.0f -y2=%.0f ", COM_CREATEPOLYGON_CMD,
				batch->polygon_name, batch->xx1,batch->yy1, batch->xx2,batch->yy2);			
			mesg->Dump();
			
			if (triangulation) {
				mesg->WriteNextLine(LOG_FATAL,"%s called after triangulation.", COM_CREATEPOLYGON_CMD);
				DumpExit();
			}
			if (!heightmap) {
				mesg->WriteNextLine(LOG_FATAL,"No heightmap present.");
				DumpExit();
			}
			polygons->AddPolygon((batch->xx1),(batch->yy1),(batch->xx2),(batch->yy2),batch->polygon_name);    	    			
		}

		if (com == COM_ADDVERTEXTOPOLYGON) {
			mesg->WriteNextLine(LOG_COMMAND,"%s (%s) -x=%.0f -y=%.0f ",
				COM_ADDVERTEXTOPOLYGON_CMD, batch->polygon_name, batch->xx1,batch->yy1);	
			mesg->Dump();
			
			if (triangulation) {
				mesg->WriteNextLine(LOG_FATAL,"%s called after triangulation.", COM_ADDVERTEXTOPOLYGON_CMD);
				DumpExit();
			}
			if (!heightmap) {
				mesg->WriteNextLine(LOG_FATAL,"No heightmap present.");
				DumpExit();
			}
			polygons->AddVertexToPolygon((batch->xx1),(batch->yy1),batch->polygon_name);    	    			
		}

		if (com == COM_READPOLYGONDATAFILE) {
			if (!batch->polygon_name)
				mesg->WriteNextLine(LOG_COMMAND,"%s -filename=%s", COM_READPOLYGONDATAFILE_CMD, batch->datafile);			
			else
				mesg->WriteNextLine(LOG_COMMAND,"%s -filename=%s -name=%s", COM_READPOLYGONDATAFILE_CMD, batch->datafile, batch->polygon_name);	
			mesg->Dump();
			
			if (triangulation) {
				mesg->WriteNextLine(LOG_FATAL,"%s called after triangulation.", COM_READPOLYGONDATAFILE_CMD);
				DumpExit();
			}
			if (!heightmap) {
				mesg->WriteNextLine(LOG_FATAL,"No heightmap present.");
				DumpExit();
			}
			if (!strlen(batch->datafile)) {
				mesg->WriteNextLine(LOG_FATAL,"No file name.");
				DumpExit();
			}
			
			FILE * file;
			if (fopen_s(&file,batch->datafile,"r")) {
				mesg->WriteNextLine(LOG_ERROR,"Unable to open %s", batch->datafile);
			} else {
				char line[1000];
				char *linenew;
				char *current_polygon = NULL;
				int num_vertex=0;
				float x1,y1,x2,y2;
				while (fgets(line,1000,file)) {
					linenew = line;
					utils->StripSpaces(&linenew);
					if (strlen(linenew)) {
						if (linenew[0] == '[') {
							//new polygon name
							for (unsigned int i=1;i<strlen(linenew);i++) if (linenew[i] == ']') linenew[i] ='\0';
							if (current_polygon) delete [] current_polygon;
							current_polygon = new char[strlen(linenew+1)+1];
							strcpy_s(current_polygon,strlen(linenew+1)+1,linenew+1);
							num_vertex=0;
						} else if (linenew[0] != '#' && linenew[0] != ';') {
							//read coordinates
							if (!current_polygon) {
								mesg->WriteNextLine(LOG_ERROR,"%s: no polygon section for [%s]", COM_READPOLYGONDATAFILE_CMD, linenew);
							} else {
								if (!batch->polygon_name || _stricmp(batch->polygon_name,current_polygon)==0) {
									num_vertex++;
									if (num_vertex == 1) {
										sscanf_s(linenew,"%f %f",&x1,&y1);
									} else if (num_vertex == 2) {
										sscanf_s(linenew,"%f %f",&x2,&y2);
										polygons->AddPolygon(x1,y1,x2,y2,current_polygon);    	    		
									} else {
										sscanf_s(linenew,"%f %f",&x1,&y1);
										polygons->AddVertexToPolygon(x1,y1,current_polygon);    
									}
								}
							}
						}
					}
				} //while
				if (current_polygon) delete [] current_polygon;
			} //fopen
		}

		if (com == COM_INACTIVATEALLVERTICES) {
			mesg->WriteNextLine(LOG_COMMAND,"%s", COM_INACTIVATEALLVERTICES_CMD);		
			mesg->Dump();
			for (int j=0;j<points->GetN();j++) {
				points->SetInactive(j);
			}
		}

		if (com == COM_ACTIVATEVISIBLEVERTICES) {
			if (batch->zz1<-999999 )  {
				mesg->WriteNextLine(LOG_WARNING,"%s: -z not set, I take the height.", COM_ACTIVATEVISIBLEVERTICES_CMD);
				batch->zz1 = heightmap->GetZ(batch->xx1,batch->yy1);
			}
			int n_rounds = 1;
			if (batch->OptRadius > 0) {
				n_rounds = 1 + 6;
				mesg->WriteNextLine(LOG_COMMAND,"%s -x=%.0f -y=%.0f -z=%.0f -radius=%.0f", COM_ACTIVATEVISIBLEVERTICES_CMD,
					batch->xx1, batch->yy1, batch->zz1, batch->OptRadius);		
			} else {
				mesg->WriteNextLine(LOG_COMMAND,"%s -x=%.0f -y=%.0f -z=%.0f", COM_ACTIVATEVISIBLEVERTICES_CMD,
					batch->xx1,batch->yy1,batch->zz1);		
			}
			mesg->Dump();
			
			if (!triangulation) {
				mesg->WriteNextLine(LOG_FATAL,"%s called without triangulation. Call MakeTriangulation before", COM_ACTIVATEVISIBLEVERTICES_CMD);
				DumpExit();
			}
			if (!heightmap) {
				mesg->WriteNextLine(LOG_FATAL,"No heightmap present.");
				DumpExit();
			}
			
			int v = triangles->GetN();
			unsigned int total_steps = n_rounds * v;
			unsigned int last_step = 0;
			for (int round=0; round < n_rounds; round++) {
				for (int i=0;i<v;i++) {

					int o1 = triangles->GetPoint1(i);
					int o2 = triangles->GetPoint2(i);
					int o3 = triangles->GetPoint3(i);

					unsigned int current_step = (round * v) + i;
					if ( ((current_step * 100) / total_steps) != last_step) {
						last_step = (current_step * 100) / total_steps;
						mesg->WriteNextLine(LOG_INFO,"%i percent complete",last_step);
						mesg->Dump();
					}

					for (int j=0;j<points->GetN();j++) {
						if (j!=o1 && j!=o2 && j!=o3) {
							float vx = points->GetX(j) - batch->xx1;
							float vy = points->GetY(j) - batch->yy1;
							float vz = points->GetZ(j) - batch->zz1;
							float s,u,v;
							float sx = batch->xx1;
							float sy = batch->yy1;
							float sz = batch->zz1;

							if (round == 1) {
								sx +=batch->OptRadius;
							} else if (round == 2) {
								sx -=batch->OptRadius;
							} else if (round == 3) {
								sy +=batch->OptRadius;
							} else if (round == 4) {
								sy -=batch->OptRadius;
							} else if (round == 5) {
								sz +=batch->OptRadius;
							} else if (round == 6) {
								sz -=batch->OptRadius;
							} 

							if (sz < heightmap->GetZ(sx,sy)) {
								sz = heightmap->GetZ(sx,sy);
							}
							
							if (!points->GetActive(j)) {
								//skip active points, they don't need to be checked again
								//because they are visible from "somewhere"
								if (points->VectorIntersectsWithTriangle(sx,sy,sz,
									vx,vy,vz,points->GetX(o1),points->GetY(o1),points->GetZ(o1),
									points->GetX(o2),points->GetY(o2),points->GetZ(o2),
									points->GetX(o3),points->GetY(o3),points->GetZ(o3),&s,&u,&v)) {
										points->SetTmpInactive(j);
								}
							} 
						}
					}
				}
				points->ClearTmpInactive();
			} //rounds			
		}

		if (com == COM_REMOVEINACTIVETRIANGLES) {
			mesg->WriteNextLine(LOG_COMMAND,"%s", COM_REMOVEINACTIVETRIANGLES_CMD);	
			mesg->Dump();

			if (!triangulation) {
				mesg->WriteNextLine(LOG_FATAL,"%s called without triangulation. Call MakeTriangulation before", COM_REMOVEINACTIVETRIANGLES_CMD);
				DumpExit();
			}
			if (!heightmap) {
				mesg->WriteNextLine(LOG_FATAL,"No heightmap present.");
				DumpExit();
			}
			int v = triangles->GetN();
			int num=0;
			for (int i=0;i<v;i++) {

				int o1 = triangles->GetPoint1(i);
				int o2 = triangles->GetPoint2(i);
				int o3 = triangles->GetPoint3(i);

				if (!points->GetActive(o1) && !points->GetActive(o2) && !points->GetActive(o3)) {
					triangles->RemoveTriangle(i);
					i--;v--;num++;
				}
			}
			mesg->WriteNextLine(LOG_INFO,"%s: %i triangles removed", COM_REMOVEINACTIVETRIANGLES_CMD, num);
		}

		if (com == COM_TRIANGULATION) {
			mesg->WriteNextLine(LOG_COMMAND,"%s: Start Delaunay triangulation", COM_TRIANGULATION_CMD);
			mesg->Dump();

			if (triangulation) {
				mesg->WriteNextLine(LOG_FATAL,"Triangulation already done.");
				DumpExit();
			}
			if (!heightmap) {
				mesg->WriteNextLine(LOG_FATAL,"No heightmap present.");
				DumpExit();
			}
			int good_flag=1;

			struct triangulateio in, mid, vorout;
			in.numberofpoints = points->GetN();
			in.numberofpointattributes = 0;
			in.pointlist = (REAL *) malloc(in.numberofpoints * 2 * sizeof(REAL));
			in.pointattributelist = (REAL *) malloc(in.numberofpoints *
								in.numberofpointattributes *
								sizeof(REAL));
			in.pointmarkerlist = (int *) malloc(in.numberofpoints * sizeof(int));
			in.numberofsegments = 0;
			in.numberofholes = 0;
			in.numberofregions = 0;
			in.regionlist = (REAL *) malloc(in.numberofregions * 4 * sizeof(REAL));
			
			int rnum=0;
			for (int i = 0; i < points->GetN(); i++) {
				in.pointlist[rnum*2] = double(points->GetX(i));
				in.pointlist[rnum*2+1] = double(points->GetY(i));
				in.pointmarkerlist[rnum] = i;
				rnum++;
			}
			in.numberofpoints = rnum;
			
			mesg->WriteNextLine(LOG_INFO,"Number of vertices forwarded to Delaunay algorithm: %i",rnum);

			mid.pointlist = (REAL *) NULL;            /* Not needed if -N switch used. */
			/* Not needed if -N switch used or number of point attributes is zero: */
			mid.pointattributelist = (REAL *) NULL;
			mid.pointmarkerlist = (int *) NULL; /* Not needed if -N or -B switch used. */
			mid.trianglelist = (int *) NULL;          /* Not needed if -E switch used. */
			/* Not needed if -E switch used or number of triangle attributes is zero: */
			mid.triangleattributelist = (REAL *) NULL;
			mid.neighborlist = (int *) NULL;         /* Needed only if -n switch used. */
			/* Needed only if segments are output (-p or -c) and -P not used: */
			mid.segmentlist = (int *) NULL;
			/* Needed only if segments are output (-p or -c) and -P and -B not used: */
			mid.segmentmarkerlist = (int *) NULL;
			mid.edgelist = (int *) NULL;             /* Needed only if -e switch used. */
			mid.edgemarkerlist = (int *) NULL;   /* Needed if -e used and -B not used. */

			vorout.pointlist = (REAL *) NULL;        /* Needed only if -v switch used. */
			/* Needed only if -v switch used and number of attributes is not zero: */
			vorout.pointattributelist = (REAL *) NULL;
			vorout.edgelist = (int *) NULL;          /* Needed only if -v switch used. */
			vorout.normlist = (REAL *) NULL;         /* Needed only if -v switch used. */

			/* Triangulate the points.  Switches are chosen to read and write a  */
			/*   PSLG (p), preserve the convex hull (c), number everything from  */
			/*   zero (z), assign a regional attribute to each element (A), and  */
			/*   produce an edge list (e), a Voronoi diagram (v), and a triangle */
			/*   neighbor list (n).   */
			mesg->WriteNextLine(LOG_INFO,"Call Triangle algorithm");
			mesg->WriteNextLine(LOG_INFO,"(written by by J. R. Shewchuk, see README)");
			mesg->Dump();
			//triangulate("pczAevn", &in, &mid, &vorout);
			triangulate("-YY -D -z -A -v -S0", &in, &mid, &vorout);

			triangulation = 1;

			int ntri = mid.numberoftriangles;

			mesg->WriteNextLine(LOG_INFO,"Number of triangles in entire worldspace: %i",ntri);

			for (int i = 0; i < ntri; i++) {
				int new_num1 = mid.trianglelist[i * 3];
				int new_num2 = mid.trianglelist[i * 3 + 1];
				int new_num3 = mid.trianglelist[i * 3 + 2];

				if (triangles->AddTriangle(new_num1, new_num2, new_num3) == -1) good_flag=0;
			}

			if (!good_flag) {
				mesg->WriteNextLine(LOG_ERROR,"Triangulation (partly) failed.");
			}
		}

		if (com == COM_WRITEQUAD || com == COM_WRITEALLQUADS) {
			mesg->WriteNextLine(LOG_COMMAND,"%s", batch->CurrentCommand);
			mesg->Dump();

			if (!triangulation && !batch->writenormalmap && !batch->writeheightmap) {
				mesg->WriteNextLine(LOG_FATAL,"%s called but no triangulation. Call %s before", batch->CurrentCommand, COM_TRIANGULATION_CMD);
				DumpExit();
			}
			if (!heightmap) {
				mesg->WriteNextLine(LOG_FATAL,"No heightmap present.");
				DumpExit();
			}
			if (com == COM_WRITEALLQUADS) quads->Reset();
			//if (heightmap) {delete heightmap;heightmap=NULL;}
			if (der && heightmap != der)       {
				delete der;
				der=heightmap;
			}
			//BUGBUG: habe to make sure that nothing else is called

writequadsloop:

			mesg->Dump();

			if (com == COM_WRITEALLQUADS) {
				batch->x00 = float(quads->GetCurrentX())*batch->cellsize_x*batch->quadsize_x;
				batch->x11 = (float(quads->GetCurrentX())+1.f)*batch->cellsize_x*batch->quadsize_x;
				batch->y00 = float(quads->GetCurrentY())*batch->cellsize_y*batch->quadsize_y;
				batch->y11 = (float(quads->GetCurrentY())+1.f)*batch->cellsize_y*batch->quadsize_y;
				batch->quadx = float(quads->GetCurrentX());
				batch->quady = float(quads->GetCurrentY());
			}

			char filename[1000];
			char filename1[1000];
			char filename2[1000];

			if (!batch->writenormalmap && !batch->writeheightmap) {
				char * texname=NULL;

				if (strlen(batch->texname)) {
					texname = batch->texname;
					if (strlen(batch->install_dir))
						sprintf_s(filename,1000,"%s\\%s.%02i.%02i.32_tex.nif",batch->install_dir,batch->worldname,(int)batch->quadx*32,(int)batch->quady*32);
					else
						sprintf_s(filename,1000,"%s.%02i.%02i.32_tex.nif",batch->worldname,(int)batch->quadx*32,(int)batch->quady*32);
				} else {
					if (strlen(batch->install_dir))
						sprintf_s(filename,1000,"%s\\%s.%02i.%02i.32.nif",batch->install_dir,batch->worldname,(int)batch->quadx*32,(int)batch->quady*32);
					else
						sprintf_s(filename,1000,"%s.%02i.%02i.32.nif",batch->worldname,(int)batch->quadx*32,(int)batch->quady*32);
				}
				WriteNif(points, triangles, heightmap,
					batch->x00,  batch->x11,  batch->y00,  batch->y11,filename, batch,
					texname,batch->ps,batch->createpedestals,batch->useshapes);
			} else {
				
					if (batch->writenormalmap) {
						if (strlen(batch->install_dir))
							sprintf_s(filename2,1000,"%s\\%s.%02i.%02i.32_fn.dds",batch->install_dir,batch->worldname,(int)batch->quadx*32,(int)batch->quady*32);
						//else 
						sprintf_s(filename,1000,"%s.%02i.%02i.32_fn.bmp",batch->worldname,(int)batch->quadx*32,(int)batch->quady*32);
						sprintf_s(filename1,1000,"%s.%02i.%02i.32_fn.dds",batch->worldname,(int)batch->quadx*32,(int)batch->quady*32);
					}
					else {
						if (strlen(batch->install_dir))
							sprintf_s(filename2,1000,"%s\\%s.%02i.%02i.32_height.dds",batch->install_dir,batch->worldname,(int)batch->quadx*32,(int)batch->quady*32);
						//else
						sprintf_s(filename,1000,"%s.%02i.%02i.32_height.bmp",batch->worldname,(int)batch->quadx*32,(int)batch->quady*32);
						sprintf_s(filename1,1000,"%s.%02i.%02i.32_height.dds",batch->worldname,(int)batch->quadx*32,(int)batch->quady*32);
					}
					if (strlen(batch->install_dir)) {
						remove (filename);
					}
					remove (filename1);
					int bmp_done = WriteBMP(heightmap, batch->x00,  batch->x11,  batch->y00,  batch->y11,filename, batch);
					if (strlen(batch->install_dir) && bmp_done) {
						remove (filename2);
						if ((rename (filename1, filename2)) != 0) {
							mesg->WriteNextLine(LOG_ERROR,"Could not move %s to %s",filename1,filename2);
						}						
						
					}
					if (bmp_done) remove (filename);
					
			}
		
			if (com == COM_WRITEALLQUADS) if (quads->GetNextQuad()) goto writequadsloop;

		}

		if (com == COM_WRITEALL) {
			mesg->WriteNextLine(LOG_COMMAND,"%s", batch->CurrentCommand);
			mesg->Dump();

			if (!triangulation && !batch->writenormalmap && !batch->writeheightmap) {
				mesg->WriteNextLine(LOG_FATAL,"%s called but no triangulation. Call %s before", batch->CurrentCommand, COM_TRIANGULATION_CMD);
				DumpExit();
			}
			if (!heightmap) {
				mesg->WriteNextLine(LOG_FATAL,"No heightmap present.");
				DumpExit();
			}
			char * texname=NULL;
			char filename[1000];

			if (!batch->writeheightmap && !batch->writenormalmap) {

				if (strlen(batch->texname) == 0) {
					if (strlen(batch->optname)) {
						sprintf_s(filename,1000,"%s",batch->optname);
					} else {
						if (strlen(batch->install_dir))
							sprintf_s(filename,1000,"%s\\%s.nif",batch->install_dir,utils->GetValue("_worldspace"));
						else
							sprintf_s(filename,1000,"%s.nif",batch->worldname);
					}
				} else {
					texname = batch->texname;
					if (strlen(batch->optname)) {
						sprintf_s(filename,1000,"%s",batch->optname);
					} else {
						if (strlen(batch->install_dir))
							sprintf_s(filename,1000,"%s\\%s_tex.nif",batch->install_dir,utils->GetValue("_worldspace"));
						else
							sprintf_s(filename,1000,"%s_tex.nif",batch->worldname);
					}
				}

				WriteNif(points, triangles, heightmap,
					float(batch->x1),  float(batch->x2),  float(batch->y1),  float(batch->y2),filename, batch,
					texname,batch->ps,batch->createpedestals,batch->useshapes);
				mesg->Dump();

			} else if (batch->writeheightmap) {
				if (strlen(batch->optname)) {
					sprintf_s(filename,1000,"%s",batch->optname);
				} else {
					if (strlen(batch->install_dir))
						sprintf_s(filename,1000,"%s\\world.bmp",batch->install_dir);
					else
						sprintf_s(filename,1000,"world.bmp");
				}
				WriteBMP(heightmap, float(batch->x1),  float(batch->x2),  float(batch->y1),  float(batch->y2), filename, batch, 0);			
			} else if (batch->writenormalmap) {
				char filename[1000];
				char filename1[1000];
				char filename2[1000];
				if (strlen(batch->install_dir))
					sprintf_s(filename2,1000,"%s\\%sMap_fn.dds",batch->install_dir,utils->GetValue("_worldspace"));
				sprintf_s(filename,1000,"%sMap_fn.bmp",utils->GetValue("_worldspace"));
				sprintf_s(filename1,1000,"%sMap_fn.dds",utils->GetValue("_worldspace"));
				if (strlen(batch->install_dir)) {
					remove (filename);
				}
				remove (filename1);
				int bmp_done = WriteBMP(heightmap, batch->x00,  batch->x11,  batch->y00,  batch->y11,filename, batch);
				if (strlen(batch->install_dir) || (strlen(batch->optname)) && bmp_done) {
					remove (filename2);
					if (strlen(batch->optname)) {
						sprintf_s(filename2,1000,"%s",batch->optname);
					} 
					if ((rename (filename1, filename2)) != 0) {
						mesg->WriteNextLine(LOG_ERROR,"Could not move %s to %s",filename1,filename2);
						mesg->Dump();
					}						
				}
				if (bmp_done) remove (filename);
			}
		}

		if ((com == COM_SETPOINTS) || (com == COM_SETPOINTSPERQUAD) || 
			(com == COM_SETMAXPOINTS) || (com == COM_SETMAXPOINTSPERQUAD)) {
				mesg->WriteNextLine(LOG_COMMAND,"%s: -n=%i", batch->CurrentCommand, batch->npoints);
				mesg->Dump();

				if (!heightmap) {
					mesg->WriteNextLine(LOG_FATAL,"No heightmap present.");
					DumpExit();
				}

				if (com == COM_SETPOINTSPERQUAD || com == COM_SETMAXPOINTSPERQUAD) quads->Reset();
setquadsloop:
				int mynum=batch->npoints;
				if (com == COM_SETMAXPOINTS) {
#if 0
					llquad * quad = 
						quads->GetQuad((batch->x00+batch->x11)/2, (batch->y00+batch->y11)/2);
					mynum -= quad->GetPoints();
#endif
					mynum -= gen_npoints;
					mesg->WriteNextLine(LOG_INFO,"Vertices left to be placed: %i",mynum);
				}

				llQuad * my_quad = NULL;
				float mean=0,num=0,num_real=0,empty=0;
				if (com == COM_SETPOINTSPERQUAD || com == COM_SETMAXPOINTSPERQUAD) {

					batch->x00 = float(quads->GetCurrentX())*batch->cellsize_x*batch->quadsize_x;
					batch->x11 = (float(quads->GetCurrentX())+1.f)*batch->cellsize_x*batch->quadsize_x;
					batch->y00 = float(quads->GetCurrentY())*batch->cellsize_y*batch->quadsize_y;
					batch->y11 = (float(quads->GetCurrentY())+1.f)*batch->cellsize_y*batch->quadsize_y;
					batch->quadx = float(quads->GetCurrentX());
					batch->quady = float(quads->GetCurrentY());

					mesg->WriteNextLine(LOG_INFO,"Current quad: x=%.0f,y=%.0f",batch->quadx,batch->quady);

					if (com == COM_SETMAXPOINTSPERQUAD) {
						my_quad = 
							quads->GetQuad((batch->x00+batch->x11)/2, (batch->y00+batch->y11)/2);
						mynum -= my_quad->GetNumPoints();
						mesg->WriteNextLine(LOG_INFO,"Vertices left to be placed: %i",mynum);
					}

					if (batch->x00<float(batch->x1)) batch->x00=float(batch->x1);
					if (batch->y00<float(batch->y1)) batch->y00=float(batch->y1);
					if (batch->x11>float(batch->x2)) batch->x11=float(batch->x2);
					if (batch->y11>float(batch->y2)) batch->y11=float(batch->y2);
					int mynum2 = int(float(mynum)*((batch->x11-batch->x00)*(batch->y11-batch->y00))/(batch->cellsize_x*batch->quadsize_x*batch->cellsize_y*batch->quadsize_y));
					if (mynum2<mynum)
						mesg->WriteNextLine(LOG_INFO,"Partly filled quad: vertices reduced to: %i",mynum2);
					mynum=mynum2;
				}

				if (alg_list.size() == 0) {
					mesg->WriteNextLine(LOG_ERROR,"%s: no algorithm specified", batch->CurrentCommand);
					goto end;
				}

				for (int num_point=0;num_point<mynum;num_point++) {	    
					int maxtry=0,maxtry_total=0;
					mesg->Dump();
					if ((num_point % 1000) == 0 && num_point) 
						mesg->WriteNextLine(LOG_INFO,"[%i]",num_point);
			
loop:	    
					float x=float((batch->x11 - batch->x00) * float(rand())/float(RAND_MAX)) + batch->x00;
					float y=float((batch->y11 - batch->y00) * float(rand())/float(RAND_MAX)) + batch->y00;

					
					float z=heightmap->GetZ(x,y);
					
					float ceiling=alg_list[0]->GetCeiling();
					float value  =alg_list[0]->GetValue(x,y);

					for (int a=1;a<alg_counter;a++) {
						alg_list[a]->GetValue(x, y, &value);
						alg_list[a]->GetCeiling(&ceiling);
					}

					if (empty>1000) {
						mesg->WriteNextLine(LOG_WARNING,"This quad seems to be empty, skipped after %i vertices",num_point);
						goto end;
					} else if (value<0.0000001) { //filter very small
						empty++;
						goto loop; 
					}

					if (heightmap->IsDefault(x,y)) //NaN
						goto loop;


					empty=0;
					mean+=value;num++;
					float idealdist=4096.f-(((4096.f-minab)/(mean/num))*value);

					if (idealdist<minab) idealdist=minab;

					if (ceiling>(10.f*mean/num)) ceiling=(10.f*mean/num);  //cutoff -> BUGBUG
					float test = float(rand())/float(RAND_MAX) * ceiling;
					if (test>value) { 
						goto loop; 
					}
					float mingrid = points->GetMinDistanceGrid(x,y);
					float maxradius = idealdist + mingrid;
					if (mingrid > minab) {
						float mindist = points->GetMinDistance(x, y, maxradius, my_quad); //time consuming!!!
						if (mindist > minab || mindist < 0) {
						//if (mindist >= 0) {

							test = 2 * float(rand())/float(RAND_MAX) * idealdist;
							if (test> (mindist + mingrid))
								goto loop; 

							maxtry=0;
							//all conditions fulfilled
							points->AddPoint(x,y,z);
							gen_npoints++;
							num_real++;
						} else { //1 -> see below
							if (maxtry<50 && maxtry_total<100*batch->npoints) {
								maxtry++;
								maxtry_total++;
								goto loop;
							} else {
								mesg->WriteNextLine(LOG_WARNING,"Mesh is too dense: quad aborted after %i vertices",num_point);
								goto end;
							}
						}
					} else { //2 -> this was done 2x on purpose to save time
						if (maxtry<50 && maxtry_total<100*batch->npoints) {
							maxtry++;
							maxtry_total++;
							goto loop;
						} else {
							mesg->WriteNextLine(LOG_WARNING,"Mesh is too dense: quad aborted after %i vertices",num_point);
							goto end;
						}
					}
				}
end:
				mesg->Dump();
				if (com == COM_SETPOINTSPERQUAD || com == COM_SETMAXPOINTSPERQUAD) if (quads->GetNextQuad()) goto setquadsloop;
		}

		llAlg * alg = NULL;

		if (com == COM_ALGCONST) {
			mesg->WriteNextLine(LOG_ALGORITHM,"%s: -add=%f -multiply=%f ", COM_ALGCONST_CMD, batch->add, batch->multiply);
			mesg->Dump();

			if (!heightmap) {
				mesg->WriteNextLine(LOG_FATAL,"No heightmap present.");
				DumpExit();
			}
			
			alg = new llAlgConst(heightmap, batch->x00, batch->y00,
				batch->x11, batch->y11);
			alg->add=batch->add;alg->multiply=batch->multiply;
			alg_list[alg_counter++] = alg;
			if (alg_counter == alg_list.size()) alg_list.resize(alg_counter + 100);
		}

		if (com == COM_ALG1ST) {
			mesg->WriteNextLine(LOG_ALGORITHM,"%s: -add=%f -multiply=%f ", COM_ALG1ST_CMD, batch->add, batch->multiply);
			mesg->Dump();

			if (!heightmap) {
				mesg->WriteNextLine(LOG_FATAL,"No heightmap present.");
				DumpExit();
			}
			
			alg = new llAlgFirst(der, batch->x00, batch->y00,
				batch->x11, batch->y11);
			alg->add=batch->add;alg->multiply=batch->multiply;
			alg_list[alg_counter++] = alg;
			if (alg_counter == alg_list.size()) alg_list.resize(alg_counter + 100);
		}

		if (com == COM_ALG2ND) {
			mesg->WriteNextLine(LOG_ALGORITHM,"%s: -add=%f -multiply=%f ", COM_ALG2ND_CMD, batch->add, batch->multiply);
			mesg->Dump();

			if (!heightmap) {
				mesg->WriteNextLine(LOG_FATAL,"No heightmap present.");
				DumpExit();
			}
			
			alg = new llAlgSecond(der, batch->x00, batch->y00,
				batch->x11, batch->y11);
			alg->add=batch->add;alg->multiply=batch->multiply;
			alg_list[alg_counter++] = alg;
			if (alg_counter == alg_list.size()) alg_list.resize(alg_counter + 100);
		}

		if (com == COM_ALGSLOPE ) {
			mesg->WriteNextLine(LOG_ALGORITHM,"%s: -add=%f -multiply=%f -lowest=%.0f -highest=%.0f -minval=%.0f -maxval=%.0f",
				COM_ALGSLOPE_CMD, batch->add, batch->multiply, batch->Lowest, batch->Highest, batch->ValueAtLowest, batch->ValueAtHighest);
			mesg->Dump();

			if (!heightmap) {
				mesg->WriteNextLine(LOG_FATAL,"No heightmap present.");
				DumpExit();
			}
			
			llAlgSlope *alg2 = new llAlgSlope(der, batch->x00, batch->y00,
				batch->x11, batch->y11);
			alg2->add=batch->add;alg2->multiply=batch->multiply;
			alg2->Lowest=batch->Lowest;
			alg2->Highest=batch->Highest;
			alg2->ValueAtLowest=batch->ValueAtLowest;
			alg2->ValueAtHighest=batch->ValueAtHighest;
			alg_list[alg_counter++] = alg2;
			if (alg_counter == alg_list.size()) alg_list.resize(alg_counter + 100);
		}

		if (com == COM_ALGRADIAL ) {
			mesg->WriteNextLine(LOG_ALGORITHM,"%s: -add=%f -multiply=%f -near=%.0f -far=%.0f -minval=%.0f -maxval=%.0f -x=%.0f -y=%.0f",
				COM_ALGRADIAL_CMD, batch->add, batch->multiply, batch->Lowest, batch->Highest, batch->ValueAtLowest,
				batch->ValueAtHighest, batch->xx1,batch->yy1);
			mesg->Dump();

			if (!heightmap) {
				mesg->WriteNextLine(LOG_FATAL,"No heightmap present.");
				DumpExit();
			}
			
			llAlgRadial *alg2 = new llAlgRadial(der, batch->x00, batch->y00,
				batch->x11, batch->y11);
			alg2->add=batch->add;alg2->multiply=batch->multiply;
			alg2->Near=batch->Lowest;
			alg2->Far=batch->Highest;
			alg2->ValueAtNear=batch->ValueAtLowest;
			alg2->ValueAtFar=batch->ValueAtHighest;
			alg2->X=batch->xx1;
			alg2->Y=batch->yy1;
			alg_list[alg_counter++] = alg2;
			if (alg_counter == alg_list.size()) alg_list.resize(alg_counter + 100);
		}

		if ( com == COM_ALGSTRIPE) {
			mesg->WriteNextLine(LOG_ALGORITHM,"%s: -add=%f -multiply=%f -lowest=%.0f -highest=%.0f -minval=%.0f -maxval=%.0f",
				COM_ALGSTRIPE_CMD, batch->add, batch->multiply, batch->Lowest, batch->Highest, batch->ValueAtLowest, batch->ValueAtHighest);
			mesg->Dump();

			if (!heightmap) {
				mesg->WriteNextLine(LOG_FATAL,"No heightmap present.");
				DumpExit();
			}
			
			llAlgStripe *alg2 = new llAlgStripe(der, batch->x00, batch->y00,
				batch->x11, batch->y11);
			alg2->add=batch->add;alg2->multiply=batch->multiply;
			alg2->Lowest=batch->Lowest;
			alg2->Highest=batch->Highest;
			alg2->ValueAtLowest=batch->ValueAtLowest;
			alg2->ValueAtHighest=batch->ValueAtHighest;
			alg_list[alg_counter++] = alg2;
			if (alg_counter == alg_list.size()) alg_list.resize(alg_counter + 100);
		}

		if ( com == COM_ALGPEAKFINDER) {
			mesg->WriteNextLine(LOG_ALGORITHM,"%s: -add=%f -multiply=%f -lowest=%.0f radius=%.0f -scanradius=%.0f -minval=%.0f -maxval=%.0f",
				COM_ALGPEAKFINDER_CMD, batch->add, batch->multiply, batch->Lowest, batch->Radius, batch->Scanradius, batch->ValueAtLowest, batch->ValueAtHighest);
			mesg->Dump();

			if (!heightmap) {
				mesg->WriteNextLine(LOG_FATAL,"No heightmap present.");
				DumpExit();
			}
			
			llAlgPeakFinder *alg2 = new llAlgPeakFinder(der, batch->x00, batch->y00,
				batch->x11, batch->y11);
			alg2->add=batch->add;alg2->multiply=batch->multiply;
			alg2->Lowest=batch->Lowest;
			alg2->linear=batch->linear;
			alg2->Radius=batch->Radius;
			alg2->Scanradius=batch->Scanradius;
			alg2->ValueAtLowest=batch->ValueAtLowest;
			alg2->ValueAtHighest=batch->ValueAtHighest;
			alg2->Init();
			alg_list[alg_counter++] = alg2;
			if (alg_counter == alg_list.size()) alg_list.resize(alg_counter + 100);
		}

		} catch (char * str) {
			if (batch->CurrentCommand)
				mesg->WriteNextLine(LOG_FATAL,"Catched exception [%s] in [%s]", str, batch->CurrentCommand);
			else 
				mesg->WriteNextLine(LOG_FATAL,"Catched exception [%s]", str);
			DumpExit();
		} catch (int str) {
			if (batch->CurrentCommand)
				mesg->WriteNextLine(LOG_FATAL,"Catched exception [%i] in [%s]", str, batch->CurrentCommand);
			else
				mesg->WriteNextLine(LOG_FATAL,"Catched exception [%i]", str);
			DumpExit();
		} catch (...) {
			if (batch->CurrentCommand)
				mesg->WriteNextLine(LOG_FATAL,"Catched exception in [%s]", batch->CurrentCommand);
			else
				mesg->WriteNextLine(LOG_FATAL,"Catched exception");
			DumpExit();
		}

		mesg->Dump();

		FILETIME userTime_old = userTime;

		res = GetSystemTimes( &idleTime, &kernelTime, &userTime );

		BOOL found = false;
		for (unsigned int ii=0; ii < time_statistics_pointer;ii++) {
			if (com == time_statistics_cmd[ii]) {
				ULARGE_INTEGER u1 = { userTime.dwLowDateTime, userTime.dwHighDateTime }; 
				ULARGE_INTEGER u2 = { userTime_old.dwLowDateTime, userTime_old.dwHighDateTime }; 
				time_statistics[ii] = u1.QuadPart - u2.QuadPart;
				found = true;
			}
		}
		if (!found && batch->CurrentCommand) {
			ULARGE_INTEGER u1 = { userTime.dwLowDateTime, userTime.dwHighDateTime }; 
			ULARGE_INTEGER u2 = { userTime_old.dwLowDateTime, userTime_old.dwHighDateTime }; 
			time_statistics[time_statistics_pointer] = u1.QuadPart - u2.QuadPart;
			time_statistics_cmd[time_statistics_pointer] = com;
			time_statistics_cmdname[time_statistics_pointer] = new char[strlen(batch->CurrentCommand)+1];
			strcpy_s(time_statistics_cmdname[time_statistics_pointer],strlen(batch->CurrentCommand)+1,batch->CurrentCommand);
			time_statistics_pointer++;
		}
	}

	std::cout << "****** Batch loop done ******" << std::endl;

	for (unsigned int ii=0; ii < time_statistics_pointer;ii++) {
		for (unsigned int jj=0; jj < time_statistics_pointer - 1;jj++) {
			if (time_statistics[jj] < time_statistics[jj+1]) {
				__int64 time_statistics_tmp = time_statistics[jj];
				char * time_statistics_cmdname_tmp = time_statistics_cmdname[jj];
				time_statistics[jj] = time_statistics[jj+1];
				time_statistics_cmdname[jj] = time_statistics_cmdname[jj+1];
				time_statistics[jj+1] = time_statistics_tmp;
				time_statistics_cmdname[jj+1] = time_statistics_cmdname_tmp;
			}
		}
	}

	std::cout << "User time per command (sorted):" << std::endl;
	for (unsigned int ii=0; ii < time_statistics_pointer;ii++) {
		if (time_statistics[ii] > 1000000)
			cout << time_statistics_cmdname[ii] << ": " << (((double)time_statistics[ii]) /10000000.)<< " s" << endl;
	}

	return 0;

}


#include <iostream>
#include <math.h>

#include <windows.h>
#include <direct.h>
#include <winreg.h>


//#include <windowsx.h>
//#include <d3d9.h>

#define REAL double
#include "../../lltool/externals/triangle/triangle.h"

#include "../../lltool/include/llcreateworkers.h"

#include "../include/llparsemodlist.h"
#include "../include/llexportmeshtonif.h"


//#define USE_CATCH 


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



#if 0
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
#endif

//************************************************************************************

int main(int argc, char **argv) {

    //FILE *fptr;
    	
	llLogger   *mesg  = _llLogger();
	llUtils    *utils = _llUtils();
	llCommands *batch = new llCommands();
	utils->SetValue("_worldspace", "Tamriel");
	//utils->SetValue("_dds_tool",   "S3TC.EXE");

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

	char *list_string = NULL;

    //******************
    //read the arguments
    //******************

	if (argc<2) {
		usage();
		DumpExit();
	}

    char * batchname = argv[argc-1];

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

				mesg->WriteNextLine(-LOG_INFO,"[Opt -f] Flag: %s",my_flag_list);
			}
			i++;
		}

		if (strcmp(argv[i],"-l")==0) {
			list_string = argv[i+1];
			mesg->WriteNextLine(-LOG_INFO, "[Opt -l] Mod list: %s", list_string); 
			i++;
		}

		if (strcmp(argv[i],"-w")==0) {
			utils->SetValue("_worldspace", argv[i+1]);
			mesg->WriteNextLine(-LOG_INFO, "[Opt -w] Worldspace: %s", utils->GetValue("_worldspace"));
		}
	}

	mesg->WriteNextLine(-LOG_INFO, "[Opt] Batchfile: %s", batchname); 
			

#if 0
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
#endif
	mesg->Dump();
	CreateWorkers(batch);
	mesg->Dump();
	if (list_string) _llUtils()->SetValue("_modlist", list_string);

	batch->RegisterWorker(new llParseModList());
	batch->RegisterWorker(new llExportMeshToNif());

	//******************
    //open the batch
	//******************
	if (batchname) {
		if (!batch->Open(batchname, "[tes4ll]")) DumpExit();
		batch->ReadCache();
		batch->CompileScript();
	} else {
		batch->ReadStdin("[tes4ll]");
		batch->ReadCache();
		batch->CompileScript();
	}

	//float minab=256;
	_llUtils()->SetValue("_mindistance", "256");
	_llUtils()->SetValue("_cellsize_x", "4096");
	_llUtils()->SetValue("_cellsize_y", "4096");

	_llUtils()->SetValue("_quadsize_x", "4096");
	_llUtils()->SetValue("_quadsize_y", "4096");
	_llUtils()->SetValue("_quad_levels", "1");

	_llUtils()->SetValue("_dds_tool", "s3tc.exe");


#ifdef _MSC_VER
	__int64 time_statistics[LLCOM_MAX_WORKERS];
	int time_statistics_cmd[LLCOM_MAX_WORKERS];
	char *time_statistics_cmdname[LLCOM_MAX_WORKERS];
	unsigned int time_statistics_pointer = 0;
#endif

	//******************
	//batch loop
	//******************


	int com;

	mesg->WriteNextLine(LOG_INFO,"****** Go into batch mode ******");

	while ((com = batch->GetCommand())>-2) {
		//cout << com << endl;
#ifdef _MSC_VER
		FILETIME idleTime;
		FILETIME kernelTime;
		FILETIME userTime;
		BOOL res = GetSystemTimes( &idleTime, &kernelTime, &userTime );
#endif

		mesg->Dump();

#ifdef USE_CATCH
		try {
#endif


#ifdef USE_CATCH
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
#endif

		mesg->Dump();
#ifdef _MSC_VER
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
#endif
	}

	std::cout << "****** Batch loop done ******" << std::endl;
#ifdef _MSC_VER
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
			std::cout << time_statistics_cmdname[ii] << ": " << (((double)time_statistics[ii]) /10000000.)<< " s" << std::endl;
	}
#endif
	return 0;


}

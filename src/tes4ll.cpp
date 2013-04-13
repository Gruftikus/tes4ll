
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
#include "../include/llimportmapfrommodlist.h"
#include "../include/tes4qlod.h"




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


int main(int argc, char **argv) {

    //FILE *fptr;
    	
	llLogger   *mesg  = _llLogger();
	llUtils    *utils = _llUtils();
	llCommands *batch = new llCommands();
	
	std::cout << "Landscape LOD generator" << std::endl;
	std::cout << "Written by gruftikus@texnexus" << std::endl;
	std::cout << "V4.10, 29.12.2012" << std::endl;
    std::cout << "***********************" << std::endl;

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
			
	mesg->Dump();
	CreateWorkers(batch);
	mesg->Dump();
	if (list_string) _llUtils()->SetValue("_modlist", list_string);

	batch->RegisterWorker(new llParseModList());
	batch->RegisterWorker(new llExportMeshToNif());
	batch->RegisterWorker(new llImportMapFromModlist());
	batch->RegisterWorker(new TES4qLOD());


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

	_llUtils()->SetValue("_quadsize_x", "131072");
	_llUtils()->SetValue("_quadsize_y", "131072");
	_llUtils()->SetValue("_quad_levels", "1");

	_llUtils()->SetValue("_dds_tool", "s3tc.exe");
	_llUtils()->SetValue("_worldspace",    "Tamriel");
	_llUtils()->SetValue("_worldspace_id", "60");
	

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

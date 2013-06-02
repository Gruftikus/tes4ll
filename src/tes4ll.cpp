
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

void usage(void) {

	_llLogger()->WriteNextLine(LOG_INFO,"Usage: tes4ll flag1 flag2 ... [section] filename.mpb");
	_llLogger()->WriteNextLine(LOG_INFO,"       filename: the name of the batch script");
	_llLogger()->WriteNextLine(LOG_INFO,"       flags: the flags which are propagated to the batch script");
	_llLogger()->Dump();

}

void DumpExit(void) {		
	_llLogger()->Dump();
	exit(-1);
}


int main(int argc, char **argv) {

	llLogger   *mesg  = _llLogger();
	llUtils    *utils = _llUtils();
	llCommands *batch = new llCommands();
	
	std::cout << "Landscape LOD generator" << std::endl;
	std::cout << "Written by gruftikus@github" << std::endl;
	std::cout << "V5.xx, xx.xx.2013" << std::endl;
    std::cout << "***********************" << std::endl;

	char *list_string = NULL;

	_llUtils()->SetValue("_worldspace",    "Tamriel");


    //******************
    //read the arguments
    //******************

	if (_stricmp(argv[argc-1], "-h") == 0 || _stricmp(argv[argc-1], "--help") == 0) {
		usage();
		DumpExit();
	}

	char *batchname = NULL;

	//check if last option is a filename
	int has_eq=0, has_dot=0;
	if (argc > 1) {
		for (unsigned int i=0; i<strlen(argv[argc-1]); i++) {
			if (argv[argc-1][i] == '.') has_dot++;
			if (argv[argc-1][i] == '=') has_eq++;
		}
	}
	if (has_dot && !has_eq) batchname = argv[argc-1];

	char *section = NULL;

	int num = argc;
	if (batchname) num--;
	for (int i=1; i<num; i++) {

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
		} else if (strcmp(argv[i],"-l")==0) {
			list_string = argv[i+1];
			mesg->WriteNextLine(-LOG_INFO, "[Opt -l] Mod list: %s", list_string); 
			i++;
		} else if (strcmp(argv[i],"-w")==0) {
			utils->SetValue("_worldspace", argv[i+1]);
			mesg->WriteNextLine(-LOG_INFO, "[Opt -w] Worldspace: %s", utils->GetValue("_worldspace"));
		} else if (argv[i][0] != '[') {
			char *my_flag_list = new char[strlen(argv[i])+2];
			strcpy_s(my_flag_list, strlen(argv[i])+1, argv[i]);
			utils->AddFlag(my_flag_list);
			mesg->WriteNextLine(LOG_INFO, "Flag: %s", my_flag_list);
		} else
			section = argv[i];

	}

	if (batchname) 
		mesg->WriteNextLine(-LOG_INFO, "[Opt] Batchfile: %s", batchname); 
	else
		mesg->WriteNextLine(-LOG_INFO, "No batchfile, will read from stdin (type @end for compilation)"); 

	if (!section) section = "[tes4ll]";
			
	mesg->Dump();
	CreateWorkers(batch);
	mesg->Dump();

	if (list_string) _llUtils()->SetValue("_modlist", list_string);

	//TES-specific stuff:
	batch->RegisterWorker(new llParseModList());
	batch->RegisterWorker(new llExportMeshToNif());
	batch->RegisterWorker(new llImportMapFromModlist());
	batch->RegisterWorker(new TES4qLOD());


	//******************
	//open the batch
	//******************
	if (batchname) {
		if (!batch->Open(batchname, section)) DumpExit();
		batch->ReadCache();
		batch->CompileScript(0);
	} else {
		batch->ReadStdin(section);
		batch->ReadCache();
		batch->CompileScript(0);
	}

	//Oblivion std-values:
	_llUtils()->SetValue("_mindistance", "256");
	_llUtils()->SetValue("_cellsize_x",  "4096");
	_llUtils()->SetValue("_cellsize_y",  "4096");

	_llUtils()->SetValue("_quadsize_x",  "131072");
	_llUtils()->SetValue("_quadsize_y",  "131072");
	_llUtils()->SetValue("_quad_levels", "1");

	_llUtils()->SetValue("_dds_tool",      "s3tc.exe");
	_llUtils()->SetValue("_worldspace_id", "60");

	return batch->Loop();
}

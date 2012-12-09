/********************************************************************************
 ** TES4qLOD.c  TES4 LOD Texture Generator.
 ********************************************************************************
 **
 ** A utility for TES4: Oblivion, TES5: Skyrim, Fallout3 and FalloutNV:
 **
 ** For Skyrim/FalloutNV/Fallout3/Oblivion:
 **
 **  - Can generate a full BMP Texture map of the landscape, full BMP normal map
 **    and individual DistantLand LOD texture and normal files 
 **    (it shells out to convert to DDS using S3's free EXE).
 **    Much faster, simpler, and more reliable than the CS.
 **
 ** For Oblivion:
 **
 **  - As well as the above, can quickly generate all the VisibleWhenDistant (.lod) 
 **    files. Includes an option to create VisibleWhenDistant for all placed statics 
 **    if desired.
 **
 ************************************************************************************
 ** Coding Disclaimer: This is not a great example of coding practice! It evolved ugly 
 **                    as I learned about the various record formats; no documentation 
 **                    existed on the net about the formats (I've since added them to 
 **                    the TES4 Wiki). The code is not particularly tight or tidy or a 
 **                    recommended example of good code. :-p
 **                    It should compile right off on Linux or the free Windows Cygwin 
 **                    compiler. A few small changes get it to work on Visual C++ too.
 **                    Developed on Linux, for speed and performance.
 **                    p.s. You may need a standard working getopt library to
 **                         to compile this code (Not all Windows compilers have this).
 **                         You will also need to link it with a zlib library.
 **
 ************************************************************************************
 ** License: Consult the GNU license; you're free to redistribute and modify
 **          this code as much as you please, just give me some recognition
 **          name somewhere if you modify or re-use the code. :-)
 **
 **
 ** First Released (for Oblivion): 0.50:     March 2007
 ** This Version:                  0.58beta: December 2011
 **
 ** Paul Halliday (aka Lightwave).
 **
 ** Branch 0.58beta-g1: added option for alpha blending 
 **                     Process4LANDData partially rewritten
 **                     March 16th, 2012
 ** IF/Gruftikus (changes marked with CHANGE_IF)
 ************************************************************************************/

#include <direct.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "../externals/dirent.h"
#include <errno.h>
#include <fcntl.h>
#include <math.h>
#include "../externals/zlib/zlib.h"
//#include <unistd.h>

/* UNIX */

/* Windoze */
//#include <io.h>
//#include <fstream.h>
//#include "getopt.h"

//#include "getopt.c"
//#include <getopt.h>

#define MKDIR(a) _mkdir(a)
//#define MKDIR(a) mkdir(a, 0x777);

/***********************
 ** Function Prototypes.
 **********************/

int myisupper(int ch)
{
    return ((ch >= 'A' && ch <= 'Z') || (ch >= 0xC0 && ch <= 0xDD));
}


void ShowUsageExit(void);

int  Process4CELLData(char *r, int size);
int  Process4LANDData(char *r, int size);
int  Process4REFRData(char *r, int size);
void Log(char *info);
void WriteCellChangeList(int old_x, int old_y, int trans_x, int trans_y);
int  CheckCellChangeList(int old_x, int old_y, int *trans_x, int *trans_y);
void LogCell4(void);
int ReadLODTextures(char *filename);
int ExportTES4LandT4QLOD(char *input_esp_filename);
int Process4WRLDData(char *r, int size);
int Add4LTEXData(char *r, int size);
int StringToReverseFormID(char *formid, char *s);
int InitLoadTexture(char *filename);

int  DecompressZLIBStream(char *input, int input_size, char output[], int *output_size); 
int  CompressZLIBStream(char *input, int input_size, char output[], int *output_size, int compress_level);

int DecodeFilenames(char *s);
int CheckTESVersion(char *input_esp_filename);

int CleanUpDir(char *dirname);
int CleanUp(void);
int ParseDir(char *dirname);

int HumptyLODs(void);
int HumptyVWD(void);
int HumptyLOD(char *lod_filename, char *tmp_bmp_dir, char *fprefix, int lmin_x, int lmax_x, int lmin_y, int lmax_y, int dsize, int invert, int qual, int mode);
void WriteBMPHeader(FILE *fp_out, int sx, int sy, int bpp);
int LOD2_Partial(char *filename, int cx, int cy, int mode);

int DumpCellBMP(int cx, int cy, char vimage[136][136][3]);

enum { EXTERIOR, INTERIOR, TRUE1, FALSE1, TEXTURES, NORMALS };

int cleanup_list_x[1048576];
int cleanup_list_y[1048576];
int cleanup_list_count = 0;

struct cell_data {
	int size;
	char name[1024];
	char worldspace_name[1024];
	int worldspace_formid;
	int current_x;
	int current_y;
	int type;
	int save;
	int copy;
} cell;

struct {
        int count;
        char filename[512][256];
} input_files;

/**************************************************************************
 *  Texture files read from the BMPs stored in the TEX_INPUT_DIR directory;
 **************************************************************************/

struct {
	char rgb[2048][16][3];
	char filename[2048][1024];
	int count;
} ftex;

// Texture FormIDs read from the LTEX file.

struct {
	int count;
	int formid[2048];
	char rgb[2048][4][4][3]; // 2048 sets of 2x2 arrays (containing 3 RGB bytes).
} lod_ltex;

/***************************************************************
 * Different games and record header sizes. Oblivion uses 20, 
 * all later games to date use 24.
 **************************************************************/

char * TES_SKYRIM  =  "Skyrim";
char * TES_MORROWIND = "Morrowind";
char * TES_FALLOUT3  = "Fallout3";
char * TES_FALLOUTNV = "FalloutNV";
char * TES_OBLIVION = "Oblivion";
char * TES_UNKNOWN  = "Unknown";

#define TES4_OB_RECORD_SIZE  20
#define TES4_FA_SK_RECORD_SIZE 24

char *opt_tes_mode = NULL;
int tes_rec_offset = TES4_OB_RECORD_SIZE; 

/***************************************************************
 * The rules array is made global, only because it saves sending
 * pointers between the ReadRules and ParseRules functions.
 **************************************************************/

char rule[5][1024];

int worldspace_found = 0;

int verbose_mode = 0,      /* Command line option: Option to produce an extensive log file.             */
    in_vwd       = 0;

int min_x = 32768,
    max_x = -32768,
    min_y = 32768,
    max_y = -32768;

int opt_ltex = -1,
    opt_bmp = 0,
    opt_no_dds = 0,
	opt_no_colorlods = 0,
	opt_no_move = 0,
    opt_load_index = 0,
    opt_q = 1,
    opt_full_map = 0,
    opt_x_offset = 0,
    opt_y_offset = 0,
    opt_debug = 0,
    opt_no_vclr = 0,
    opt_lod_tex = 0,
    opt_normals = 0,
    opt_lod2 = 0,
    opt_vwd = 0,
    opt_vwd_everything = 0,
	opt_blending = 0, //CHANGE_IF
	opt_read_heightmap = 0,//CHANGE_IF
	opt_read_dimensions = 0,//CHANGE_IF
	opt_size_x = 0,
	opt_size_y = 0,
	opt_center = 0,
	opt_flip = 0,
    argn = 0;

char * opt_install_dir = NULL; //optional for tes4ll

int verbosity = 1;

FILE *fp_out;

char log_message[512];
char input_esp_filename[512*256]; //CHANGE_IF: 128 was _much_ too small
char tmp_dirname[256];
char worldspace[256];
char worldspace_lc[256];
char game_textures_filepath[256];

/***************************
* Just some running totals.
**************************/

int total_cells = 0,		    /* Total CELL records found in the file. */
    total_land = 0,		    /* Total LAND records found in the file. */
    total_records = 0,
    total_records_changed = 0,      /* LAND or CELL records changed/copied.  */
    total_cells_copied = 0,	
    total_land_copied = 0,	
    total_worlds = 0,
    total_refr = 0,
    total_vwd = 0,
    total_books = 0,      
    total_objects = 0,		    /* Total objects found in the file.      */
    total_objects_changed = 0,      /* Total objects changed/copied.   	     */
    total_scripts = 0,		    /* Total SCPT records found in the file. */
    total_scripts_changed = 0,	    /* Total SCPT records modified.          */
    total_dialogs = 0,		    /* Total INFO scripts found in the file. */
    total_dialogs_changed = 0;	    /* Total INFO records modified.          */


#define APP_NAME	"TES4QLOD"
#define APP_VERSION     "4.10"
#define APP_DATE        "11 Nov. 2012"
#define LOD2_TEX_DIR	"Textures"
#define IMPORT_TEX_DIR	"tes4qlod_tex/%s"

#define LOD_LTEX_DATA_FILE 	"tes4qlod_%s_ltex.dat"
#define LOD_OUTPUT_DIR_OBLIVION		"Textures/LandscapeLOD/Generated"
#define LOD_OUTPUT_DIR_OBLIVION_DOS	"Textures\\LandscapeLOD\\Generated"

#define LOD_OUTPUT_DIR_FALLOUT3		"Textures/Landscape/lod/%s"
#define LOD_OUTPUT_DIR_FALLOUT3_DOS	"Textures\\Landscape\\lod\\%s"

#define LOD_OUTPUT_DIR_SKYRIM		"Textures/Terrain/%s"
#define LOD_OUTPUT_DIR_SKYRIM_DOS	"Textures\\Terrain\\%s"

#define FULL_LOD_MAP		"%sMap.bmp"
#define FULL_LOD_NORMAL_MAP	"%sMap_fn.bmp"
#define TMP_TEX_DIR		"tes4qlod_partials"
#define TMP_NORMAL_DIR		"tes4qlod_normals"
#define TMP_VWD_DIR		"tes4qlod_tmp_vwd"
#define VWD_DIR			"DistantLOD"

char * DDS_CONVERTOR = "S3TC.EXE";

//#define DDS_CONVERTOR		"nconvert -out dds"
//#define DDS_CONVERTOR		"nconvert.exe -out dds"

/*********************************************************************
 ** 1. main(): The Main routine processes command line options,
 **            reads tes4qlod_GAME_ltex.dat file (contains base 
 **            LTEX (land texture FormIDs and filenames from the 
 **            original ESMs - this saves having to include the ESM
 **            each time you run this program.
 **
 **            Then calls ExportTES4LandT4QLOD() to parse the ESM/ESPs.
 **            Then calls HumptyLODs() to assemble all the temporary
 **            cell texture and normal BMP files in to quads and (if 
 **            requested) a full map. 
 **            Optionally calls HumptyVWD() to assemble the VWD records.
 *********************************************************************/

#if 1

#ifdef FROMTES4LL
int calltes4qlod(int argc, char *argv[])
#else
int main(int argc, char *argv[])
#endif
{

	int i;

	int in_group = 0,   /* Are we in a GRUP record group or not? */
//	    size,           /* Size of current record.             */
	    group_size = 0,
	    pos = 0;

	char s[40];	/* For storing the 16-byte header. */
//	char *or;	/* Pointer to the Original Record. */

//	char tmp_vwd_filename[512];
	char lod_texture_formids_file[512];

	FILE *fpin; /* Input File Stream (original ESP/ESM).      */
	/* Output FileStream (TESFAITH_NEW_ESP_FILE). */


	//tes4qlod can be called >1 times via tes4ll:
	opt_ltex = -1;
    opt_bmp = 0;
	opt_no_colorlods = 0;
    opt_no_dds = 0;
	opt_no_move = 0;
    opt_load_index = 0;
    opt_q = 1;
    opt_full_map = 0;
    opt_x_offset = 0;
    opt_y_offset = 0;
    opt_debug = 0;
    opt_no_vclr = 0;
    opt_lod_tex = 0;
    opt_normals = 0;
    opt_lod2 = 0;
    opt_vwd = 0;
    opt_vwd_everything = 0;
	opt_blending = 0;
	//opt_read_heightmap = 0; //overwritten by tes4ll
	opt_read_dimensions = 0;
    argn = 0;

	total_cells = 0;	    
    total_land = 0;	   
    total_records = 0;
    total_records_changed = 0;
    total_cells_copied = 0;
    total_land_copied = 0;
    total_worlds = 0;
    total_refr = 0;
    total_vwd = 0;
    total_books = 0;
    total_objects = 0;		   
    total_objects_changed = 0;
    total_scripts = 0; 
    total_scripts_changed = 0;
    total_dialogs = 0;
    total_dialogs_changed = 0;	  



	/*
	 * Invalid args. Try and get them from the Rules file if not on the command line.
	 */

	argn = argc;
	opt_tes_mode = TES_UNKNOWN;

	//for (i=0;i<argc;i++) printf("'%s' ",argv[i]);

	printf("################################################################################\n");
	printf("TES4QLOD: A Quick Texture and/or VWD Generator for Oblivion/Fallout/Skyrim \n          Landscape LODs\n");
	printf("          Version: %s\n",APP_VERSION);
	printf("          Date: %s\n",APP_DATE);
	printf("################################################################################\n");

	if (verbosity) printf("\n Working directory: %s\n",_getcwd( NULL, 0 )); //CHANGE_IF
	fflush(stdout); //CHANGE_IF

	/***********************************
	 * Parse the command line arguments.
	 **********************************/
	for (i=1; i<(argc-1); i++) {

		if (strcmp(argv[i],"-a")==0) {
			opt_blending = 1;
		}

		if (strcmp(argv[i],"-x")==0) {
			opt_read_dimensions = 1;
		}

		if (strcmp(argv[i],"-g")==0 || strcmp(argv[i],"-G")==0) {
			if (strcmp(argv[i+1], TES_SKYRIM) == 0) {
				opt_tes_mode = TES_SKYRIM;
				tes_rec_offset = TES4_FA_SK_RECORD_SIZE;
			} else if (strcmp(argv[i+1], TES_FALLOUT3) == 0) {
				opt_tes_mode = TES_FALLOUT3;
				tes_rec_offset = TES4_FA_SK_RECORD_SIZE;
			} else if (strcmp(argv[i+1], TES_FALLOUTNV) == 0) {
				opt_tes_mode = TES_FALLOUTNV;
				tes_rec_offset = TES4_FA_SK_RECORD_SIZE;
			} else if (strcmp(argv[i+1], TES_OBLIVION) == 0) {
				opt_tes_mode = TES_OBLIVION;
				tes_rec_offset = TES4_OB_RECORD_SIZE;
			} else	{
				ShowUsageExit();
				return -1;
			}	
		}

		if (strcmp(argv[i],"-X")==0) {
			opt_size_x = atoi(argv[i+1]);
		}

		if (strcmp(argv[i],"-z")==0) {
			opt_center = 1;
		}

		if (strcmp(argv[i],"-Y")==0) {
			opt_size_x = atoi(argv[i+1]);
		}

		if (strcmp(argv[i],"-i")==0) {
			opt_load_index = atoi(argv[i+1]);
		}

		if (strcmp(argv[i],"-f")==0) {
			opt_full_map = 1;
		}

		if (strcmp(argv[i],"-F")==0) {
			opt_flip = 1;
		}

		if (strcmp(argv[i],"-n")==0) {
			opt_normals = 1;
		}

		if (strcmp(argv[i],"-t")==0 || strcmp(argv[i],"-T")==0) {
			opt_normals = 1;
		}

		if (strcmp(argv[i],"-E")==0) {
			opt_vwd_everything = 1;
				opt_vwd = 1;
		}

		if (strcmp(argv[i],"-L")==0) {
			opt_lod2 = 1;
				opt_normals = 1;
		}

		if (strcmp(argv[i],"-c")==0) {
			opt_no_vclr = 1;
		}

		if (strcmp(argv[i],"-V")==0) {
			opt_vwd = 1;
		}

		if (strcmp(argv[i],"-q")==0) {
			opt_q = atoi(argv[i+1]);
			if (opt_q!=1 && opt_q!=2 && opt_q!=4) {
				ShowUsageExit();
				return -1;
			}
		}

		if (strcmp(argv[i],"-B")==0) {
			opt_no_dds = 1;
		}

		if (strcmp(argv[i],"-C")==0) {
			opt_no_colorlods = 1;
		}

		if (strcmp(argv[i],"-D")==0) {
			opt_no_move = 1;
		}

		if (strcmp(argv[i],"-d")==0) {
			opt_debug = 1;
		}

		if (strcmp(argv[i],"-v")==0) {
			printf("Program Name: %s\nVersion:      %s\nAuthor:       Paul Halliday (Ocean_Lightwave@yahoo.co.uk)\n",
				APP_NAME, APP_VERSION);
			exit(0);
		}

	}

	if (verbosity) printf("\n");

	if (argn < 3) {
		ShowUsageExit();
		return -1;
	} 

	if (opt_q < 1) opt_q = 1;
	if (opt_q == 3) opt_q = 2;
	if (opt_q > 4) opt_q = 4;

	if (opt_lod_tex == 0 && !opt_vwd && !opt_read_heightmap && !opt_read_dimensions) { opt_lod_tex = 1; }

	CleanUpDir(TMP_TEX_DIR);
	CleanUpDir(TMP_NORMAL_DIR);
	CleanUpDir(TMP_VWD_DIR);

	strcpy_s(worldspace, 256, argv[argc-2]);
	strcpy_s(input_esp_filename, 131072, argv[argc-1]);


//	MKDIR(TMP_TEX_DIR, S_IRWXU | S_IRWXG | S_IRWXO);
	if (opt_lod_tex)
		MKDIR(TMP_TEX_DIR);

	/**********************************************************************
	 * Create Final Texture directories (in case they don't exist).
	 * This is where the games will expect to find their LOD texture files.
	 **********************************************************************/
	if (opt_lod_tex) {
		if (opt_tes_mode == TES_OBLIVION) {
			MKDIR("Textures");
			MKDIR("Textures/LandscapeLOD");
			MKDIR(LOD_OUTPUT_DIR_OBLIVION);
		} else if (opt_tes_mode == TES_FALLOUT3 || opt_tes_mode == TES_FALLOUTNV) {
			MKDIR("Textures");
			MKDIR("Textures/Landscape");
			MKDIR("Textures/Landscape/lod");
			sprintf_s(tmp_dirname, 256, "Textures/Landscape/lod/%s", worldspace);
			MKDIR(tmp_dirname);
			sprintf_s(tmp_dirname, 256, "Textures/Landscape/lod/%s/diffuse", worldspace);
			MKDIR(tmp_dirname);
			sprintf_s(tmp_dirname, 256, "Textures/Landscape/lod/%s/normals", worldspace);
			MKDIR(tmp_dirname);
		} else { // Skyrim.
			MKDIR("Textures");
			MKDIR("Textures/terrain");
			sprintf_s(tmp_dirname, 256, "Textures/terrain/%s/", worldspace);
			MKDIR(tmp_dirname);
		}
	}
	if (opt_normals) {
		MKDIR(TMP_NORMAL_DIR);
	}
	if (opt_lod2) {
		MKDIR("Textures/LOD2");
		sprintf_s(worldspace, 256, "Textures/LOD2/%s", argv[argc-2]);
		MKDIR(worldspace);
	}
	if (opt_vwd) {
		MKDIR(TMP_VWD_DIR);
	}


	/*
	 * Initialize variables:
	 */

//	printf("X offset will be %d. Y will be %d\n", opt_x_offset, opt_y_offset);

	cell.name[0] = '\0';
	cell.current_x = 0;
	cell.current_y = 0;

	DecodeFilenames(argv[argc-1]);

	printf("esp-list decoded, I will run over %i files\n",input_files.count); //CHANGE_IF

	/***************************************************************************************
	 * Open the first filename argument (argv[argc-1]) - the input ESM/ESP file for reading.
	 **************************************************************************************/
	CheckTESVersion(input_files.filename[0]);

	if ((fpin = fopen(input_files.filename[0], "rb")) == NULL) {
		fprintf(stderr, "Unable to open %s for reading: %s\n",
			input_esp_filename, strerror(errno));
		exit(1);
	}

	if (fread(s, 1, 4, fpin) < 4) {
		fprintf(stderr, "Unable to read the first 4 bytes from %s to determine if it's "
			"a TES3 or TES4 file: %s\n", strerror(errno));
		exit(1);
	} else {
		if (strncmp(s, "TES3", 4) == 0) {
			printf("This is a TES3: Morrowind file: But I only decode TES4: Oblivion/Fallout3/FalloutNV/Skyrim files. I only create landscape LOD textures and VisibleWhenDistant files!\n");
			exit(1);
		} else if (strncmp(s, "TES4", 4) != 0) {
			printf("This is neither a TES3 or TES4 file - the first 4 bytes do not match what I'd expect!\n");
			exit(1);
		}
		fseek(fpin, 0, SEEK_SET);
	}

	fclose(fpin);

	/*********************************************************************************************
	 * Parse the texture directory (IMPORT_TEX_DIR) and read in all the BMP textures in to memory.
	 ********************************************************************************************/

	if (opt_lod_tex) {
		sprintf_s(game_textures_filepath, 256, IMPORT_TEX_DIR, opt_tes_mode);
		if (verbosity) printf("Caching all BMP texture files stored in the %s directory ... ", game_textures_filepath);
		fflush(stdout);

		ftex.count = 0;

		ParseDir(game_textures_filepath);

		if (verbosity) printf(" finished.\n");
	}

	/*********************************************************************************************
	 * Load the standard (Oblivion.esm) LTEX FormIDs and their filenames in to memory.
	 * The RGB data is matched up to the filenames cached from the IMPORT_TEX_DIR.
	 ********************************************************************************************/

	if (opt_lod_tex) {
		sprintf_s(lod_texture_formids_file, 512, LOD_LTEX_DATA_FILE, opt_tes_mode);
		if (verbosity) printf("Reading %s.esm Texture FormIDs from %s ...", opt_tes_mode, lod_texture_formids_file);
		fflush(stdout);


		ReadLODTextures(lod_texture_formids_file);
		if (verbosity) printf(" finished.\n\n");
	}

	if (verbosity) printf("Searching %s for any cells in the worldspace %s:\n\n", input_esp_filename, worldspace);

	for (i = 0; worldspace[i] != '\0'; i++) {
		worldspace_lc[i] = tolower(worldspace[i]);
	}
	worldspace_lc[i] = '\0';
			

	/*********************************************************
	 ** Start the ESP reading and exporting for each file.
	 *********************************************************/

	for (i = 0; i < input_files.count; i++) {
		ExportTES4LandT4QLOD(input_files.filename[i]);
	}

	if (opt_size_x) {
		if (opt_center) 
			min_x -= (opt_size_x - (max_x - min_x))/2;
		max_x = min_x + opt_size_x - 1;
	}
	if (opt_size_y) {
		if (opt_center) 
			min_y -= (opt_size_y - (max_y - min_y))/2;
		max_y = min_y + opt_size_y - 1;
	}

	if (total_cells > 0) {
		if (opt_lod_tex) {
			HumptyLODs();
		}
		if (opt_vwd) {
			HumptyVWD();
		}
	} else {
		fprintf(stderr, "No cells were found!! Did you specify the correct worldspace?! No LOD textures have been generated.\n");
		exit(0);
	}

	/******************************
	* Dump out the running totals.
	*****************************/

	if (verbosity) sprintf_s(log_message, 512, "\nWe've finished!\n\n"
		"\tTotal Worldspaces  found:                     %d\n"
		"\tTotal CELL records found:                     %d\n"
		"\tTotal LAND records found:                     %d\n"
		"\tTotal CELL records exported:                  %d\n"
		"\tTotal LAND records exported:                  %d\n"
		"\tTotal VWD  Objects written:                   %d\n",
		total_worlds, total_cells, total_land, total_cells_copied, total_land_copied, total_vwd);
	Log(log_message);
	if (verbosity) printf(log_message);

	if (opt_read_dimensions && (verbosity) ) {
		printf("\nMin X:      %d",min_x);
		printf("\nMax X:      %d",max_x);
		printf("\nMin Y:      %d",min_y);
		printf("\nMax Y:      %d",max_y);
	}

	if (!opt_debug) {
		if (verbosity) printf("\nNow I'm cleaning up my temporary files ... (sometimes takes a while, but all your files are now generated).\n");
		CleanUp();
	} else {
		if (verbosity) printf("\nYou're in \"Debug\" mode, so I won't delete my BMP files or my %s temp directory.\n", TMP_TEX_DIR);
	}

	return 0;
}

/************************************************************************
 ** ShowUsageExit: Show how to use the command line arguments, then exit.
 ***********************************************************************/
void ShowUsageExit(void)
{
	fprintf(stderr, "Program Name: %s\n"
	       		"Version:      %s\n"
			"Author:       Paul Halliday (Ocean_Lightwave@yahoo.co.uk)\n\n",
				APP_NAME, APP_VERSION);

	fprintf(stderr, "Usage: [-G Game] [-i index] [-V] [-T] [-n] [-f] [-c] [-B] [-d] WorldSpaceName ModFile.esp\n\n"
			"\tWorldSpaceName: The name of the worldspace you want to generate LOD textures for.\n"
			"\tOriginal.esp: The filename of the TES4 ESM/ESP containing your worldspace.\n\n"
			"\t-G game: Specify either Oblivion,Fallout3,FalloutNV or Skyrim. The default is Oblivion.\n"
			"\t-i (n): Specify the Load Order Index for this mod (default is 0).\n"
			"\t-f: Generate a full texture map (BMP) of the complete worldspace.\n"
			"\t-a: Use blending based on the layer's opacity\n"
			"\t-n: Generate normal _fn files.\n"
			"\t-c: Don't include the Vertex Colour Map data on the LOD textures or full map.\n"
			"\t-B: BMP mode: Create BMPs only. Do create DDS files or move them to LOD directory.\n"
			"\t-D: Create DDS files, but don't move them to LOD directory.\n"
			"\t-d: Debug mode: Do not remove the partial cell texture files or BMP LOD textures\n"
			"\t-V: Generate the VisibleWhenDistant .lod files in the DistantLOD folder.\n"
			"\t-E: Everything VWD: Make all placed objects VisibleWhenDistant (in .lod files). Implies -V.\n"
			"\t-T: Force texture generation at the same time, if -V (distantLOD) is specified.\n"
			"\t-L: Generate extures for LOD2 (VWD DistantLand replacement).\n"
			"\t-v : Display current Version, then exit.\n");

	fprintf(stderr, "\nIf you're using your own textures, copies of these should be converted in to 4x4 BMP files and placed in the tes4qlod_Game_tex directory.\n");

#ifndef FROMTES4LL
	exit(1);
#endif
}

/*******************************************************************************
 ** DecodeFilenames(): Split multiple filename command-line args in to an array.
 ******************************************************************************/

int DecodeFilenames(char *s)
{
        int i = 0,
            p = 0;

        input_files.count = 0;

        while (s[p] != '\0') {
                for (i = 0; s[i+p] != ',' && s[i+p] != '\0'; input_files.filename[input_files.count][i] = s[i+p], i++);
                input_files.filename[input_files.count][i] = '\0';
                //printf("Got %s\n", input_files.filename[input_files.count]);
                input_files.count++;

                if (s[i+p] == '\0') break;
                p += i + 1;
        }

        return 0;
}

/***************************************************************************************************
 ** CheckTESVersion(): Manually try to work out the file format (in case -G isn't specified).
 **
 ** If it's a Fallout3/NV/Skyrim file, the record size is 4 bytes longer than an Oblivion file.
 ** This tries to guess it by checking imediately after the first chunk whether it's alphanumemeric.
 **************************************************************************************************/

int CheckTESVersion(char *input_esp_filename)
{
	char s[4];
	FILE *fpin;

	if ((fpin = fopen(input_files.filename[0], "rb")) == 0) {
		fprintf(stderr, "Cannot open %s for reading: %s\n", 
			input_esp_filename, strerror(errno));
		return -1;
	}
	fread(s, 4, 1, fpin);

	if (strncmp("TES4", s, 4) == 0) {
		fseek(fpin, TES4_FA_SK_RECORD_SIZE, SEEK_SET);
		fread(s, 4, 1, fpin);
		if (isalpha(s[0]) && isalpha(s[1]) && isalpha(s[2]) && isalpha(s[3])) {
			if (verbosity) printf("Looks like a TES4 (Fallout3 / FalloutNV / Skyrim) file\n");
			if (tes_rec_offset != TES4_FA_SK_RECORD_SIZE) {
				tes_rec_offset = TES4_FA_SK_RECORD_SIZE;
			}
			if (opt_tes_mode == TES_UNKNOWN) {
				if (verbosity) printf("Going to assume Skyrim LOD type. Please override with \"-G GameType\" if this is wrong\n");

				opt_tes_mode = TES_SKYRIM;
			}

		} else {
			if (verbosity) printf("Looks like a TES4 Oblivion file.\n");
			if (tes_rec_offset != TES4_OB_RECORD_SIZE) {
				tes_rec_offset = TES4_OB_RECORD_SIZE; 
			}
			if (opt_tes_mode == TES_UNKNOWN) {
				printf("Going to assume Oblivion LOD type. Please override with \"-G GameType\" if this is wrong\n");
				opt_tes_mode = TES_OBLIVION;				
			}
		}
	}
	fclose(fpin);

	return 0;
}

/************************************************************************************************
 ** ExportTES4LANDT4QLOD(): Export the texture details and/or VWD objects from ESM/ESPs.
 **
 **  Opens the specified TES file as input, parses looking for CELL, LAND, LTEX and FRMR records, 
 **  reads each record in to memory for processingm handing them on to one of:
 **                Process4CELLData(), Process4LANDData(), 
 **                Process4LTEXData(), Process4FRMRData().
 **
 **  1. Process4CELLData() retrieves the X and Y co-ordinate of the cell.
 **  2. Process4LTEXData() retrieves the land textute FormID and filename.
 **  3. Process4LANDData() saves the normal and colour map data, and matches texture layer FormIDs
 **     with the bitmap images, producing a BMP for each cell.
 **  4. Process4FRMRData() retrieves placed object data; FormID and whether they're VWD.
 ***********************************************************************************************/

int ExportTES4LandT4QLOD(char *input_esp_filename)
{
	int //i,
	    size,	   /* Size of current record.               */
	    in_group = 0,  /* Are we in a GRUP record group or not? */
	    pos = 0,
	    group_size = 0;

//	char c;		/* For command-line getopts args.     */
	char s[40];	/* For storing the record header.     */
	char *r;	/* Pointer to the Record Data.        */

	FILE *fpin; /* Input File Stream (original ESP/ESM).  */

	CheckTESVersion(input_esp_filename);

	if ((fpin = fopen(input_esp_filename, "rb")) == NULL) {
		fprintf(stderr, "Unable to open %s for reading: %s\n",
			input_esp_filename, strerror(errno));
		exit(1);
	}

	while (fread(s, 1, 8, fpin) > 0) {

		if (!isalpha(s[0]) && !isalpha(s[1]) && !isalpha(s[2]) && !isalpha(s[3])) 
			printf(" - WARNING: FOUND A WILD NON-ASCII RECORD HEADER: %c%c%c%c ", s[0], s[1], s[2], s[3]);

		/**************************************
		 * The Core TES4 ESM/ESP Record Parser.
		 *************************************/

		if (strncmp(s, "TES4", 4) == 0 ||
			strncmp(s, "GRUP", 4) == 0) {
			size = tes_rec_offset;
		} else if ( 
			strncmp(s, "HEDR", 4) == 0 ||
			strncmp(s, "OFST", 4) == 0 ||
			strncmp(s, "MAST", 4) == 0 ||
			strncmp(s, "DATA", 4) == 0 ||
			strncmp(s, "DELE", 4) == 0 ||
			strncmp(s, "CNAM", 4) == 0 ||
			strncmp(s, "INTV", 4) == 0 ||
			strncmp(s, "SNAM", 4) == 0) {

			size = 0;
			memcpy(&size, (s+4), 2);
			size += 6;

		} else  {
			memcpy(&size, (s+4), 4);
			size += tes_rec_offset;
		}

		/************************************
		 * End of Core TES4 Parser.
		 ***********************************/

		/************************************
		 * Keep seeking to the next record if
		 * it's of no interest to us.
		 ***********************************/

		if (strncmp(s, "CELL", 4) != 0 &&
			strncmp(s, "LAND", 4) != 0 &&
			strncmp(s, "WRLD", 4) != 0 &&
			strncmp(s, "TXST", 4) != 0 &&
			strncmp(s, "LTEX", 4) != 0) {
			fseek(fpin, size-8, SEEK_CUR);
			continue;
		}

		/**********************************************
		 * It must be one we need to process, so create
		 * some memory space to store the record.
		 *********************************************/

		if ((r = (char *)(void *) malloc(size)) == NULL) { //!
			fprintf(stderr, "Unable to allocate %d bytes of memory to store TES file record: %s\n",
				2*size, strerror(errno));
			exit(1);
		}

		fseek(fpin, -8, SEEK_CUR);

		if (fread(r, 1, size, fpin) < size) {
			fprintf(stderr, "Unable to read entire marker record (%d bytes) from %s into memory: %s\n",
				size, input_esp_filename, strerror(errno));
			exit(1);
		}
		pos+= 6 + size;

		/******************************************************
		 ** If it's a CELL or a LAND record, then hand it on to
		 ** a procedure that will handle the format, including
		 ** determining if any modifications should be made.
		 *****************************************************/

		if (strncmp(s, "CELL", 4) == 0) {
			total_cells++;
			Process4CELLData(r, size);
		} else if (strncmp(s, "WRLD", 4) == 0) {
			if (!Process4WRLDData(r, size)) {
				total_worlds++;
			}
		} else if (strncmp(s, "LAND", 4) == 0) {
			total_land++;
			Process4LANDData(r, size);
		} else if (opt_lod_tex && (strncmp(s, "LTEX", 4) == 0)) { // || strncmp(s, "TXST", 4) == 0))  {
		//else if ((strncmp(s, "LTEX", 4) == 0)) { // || strncmp(s, "TXST", 4) == 0))  {
			Add4LTEXData(r, size);
		} else if (strncmp(s, "GRUP", 4) == 0) {
			if (r[12] == 0x0A) {
				in_vwd = 1;
			} else {
				in_vwd = 0;
			}
		} else if ((opt_vwd_everything || (opt_vwd && in_vwd)) && strncmp(s, "REFR", 4) == 0) {
			Process4REFRData(r, size);
		}

		free(r);
	}
	fclose(fpin);

	return 0;
}

/********************************************************************
** 0. Add4LTEXData():
** 	Processes a TES4 LTEX record, extracts the FormID and texture 
**	filename and if it doesn't currently exist in the list of
**	used FormIDs, adds it.
**	If the FormID is already used, the RGB data should be replaced
**	in memory (the user may decide to replace an OB texture with a 
**	nicer file).
*********************************************************************/

int Add4LTEXData(char *r, int size)
{
	int  i;
	int p, j, k, t;
	int  nsize;
	int  pos = 0;
	int  formid = 0;
	int  rgb[3];
//	char tmp_int[5];

	char fname[512];

	memcpy(&formid, r + 12, 4);

	pos += tes_rec_offset;

	while (pos < size) {
		nsize = 0;
		memcpy(&nsize, r+pos+4, 2);

		if (strncmp("ICON", r + pos, 4) == 0) {
			memcpy(fname, r + pos + 6, nsize);
		} else if (strncmp("TX00", r + pos, 4) == 0) {
			if (opt_tes_mode == TES_SKYRIM) {
				memcpy(fname, r + pos + 6, nsize - 4);
				fname[nsize - 4] = '\0';
				strcat_s(fname, 512, ".dds");
			}
		} else if (strncmp("EDID", r + pos, 4) == 0) {
			if (opt_tes_mode == TES_SKYRIM) {
				memcpy(fname, r + pos + 7, nsize - 1);
			} else {
				memcpy(fname, r + pos + 6, nsize);
			}
			strcat_s(fname, 512, ".dds");
		}

		pos += 6 + nsize;
	}

	/***************************************************************************
	 ** Disabled, but useful to generating that initial DAT file for a new game.
	{
		unsigned char hex_formid[4];
		memcpy(&hex_formid, &formid, 4);
		printf("DAT:%2.2X%2.2X%2.2X%2.2X,%s\n", hex_formid[3], hex_formid[2], hex_formid[1], hex_formid[0], fname);
	}
	***************************************************************************/

	if (fname[0] == '\0') {
		printf("??? This is really weird. I found an LTEX record in your ESM/ESP file with no texture filename associated with it!! Ignoring ...\n");
	}

	for (i = 0; i < lod_ltex.count; i++) {
		if (lod_ltex.formid[i] == formid) {
			break;
		}
	}
	if (i == lod_ltex.count) {
		for (t = 0; t < ftex.count; t++) {
			// printf("Comparing LTEX %s to %s\n", ftex.filename[i], fname);
			if (_stricmp(ftex.filename[t], fname) == 0) {
				break;
			}
		}
		if (t == ftex.count) {
			fprintf(stdout, "Unable to find the BMP version of the texture file %s in my textures directory (%s), but your mod file has an LTEX record that thinks it should exist. :-\\ \n",
			fname, game_textures_filepath);
		} else {
			lod_ltex.formid[lod_ltex.count] = formid;
			if (verbosity) printf("Texture %s: FormID: %d\n", fname, formid);
			for (p = 0; p < 16; p++) {
                                i = (int) p/4;
                                j = p - (4*i);
				memcpy(&lod_ltex.rgb[lod_ltex.count][i][j], &ftex.rgb[t][p][0], 3);
                        }

                        if (opt_q < 4) {
                                for (i = 0; i < 2; i++) {
                                        for (j = 0; j < 2; j++) {
                                                for (k = 0; k < 3; k++) {
                                                        rgb[k] = (unsigned char) lod_ltex.rgb[lod_ltex.count][2*i][2*j][k];
                                                        rgb[k] += (unsigned char) lod_ltex.rgb[lod_ltex.count][2*i][2*j+1][k];
                                                        rgb[k] += (unsigned char) lod_ltex.rgb[lod_ltex.count][2*i+1][2*j][k];
                                                        rgb[k] += (unsigned char) lod_ltex.rgb[lod_ltex.count][2*i+1][2*j+1][k];
                                                        lod_ltex.rgb[lod_ltex.count][i][j][k] = (unsigned char) ((float) rgb[k] / 4.0f);
                                                }
                                        }
                                }
                        }

                        if (opt_q == 1) {
                                for (i = 0; i < 2; i++) {
                                        for (j = 0; j < 2; j++) {
                                                for (k = 0; k < 3; k++) {
                                                        rgb[k] = (unsigned char) lod_ltex.rgb[lod_ltex.count][2*i][2*j][k];
                                                        rgb[k] += (unsigned char) lod_ltex.rgb[lod_ltex.count][2*i][2*j+1][k];
                                                        rgb[k] += (unsigned char) lod_ltex.rgb[lod_ltex.count][2*i+1][2*j][k];
                                                        rgb[k] += (unsigned char) lod_ltex.rgb[lod_ltex.count][2*i+1][2*j+1][k];
                                                        lod_ltex.rgb[lod_ltex.count][i][j][k] = (unsigned char) ((float) rgb[k] / 4.0f);
                                                }
                                        }
                                }
                        }

			lod_ltex.count++;
		}
	}

	return 0;
}

/***************************************************************
** 2. Process4CELLData:
** 	Processes a TES4 CELL record, extracting name, region and any
**	REFR (objects in this cell) records (if they exist).
***************************************************************
** Format:
** CELL (4) + Length (2) + intro_data (tes_rec_offset; 20 for Oblivion, 24 for FalloutNV/Skyrim)
** EDID (name 4) + Length (2) + NameAsString (size given by Length)
** XCLC (4) + Length(2) + X-co-ord (4) + Y-co-ord (4)
***************************************************************/

int Process4CELLData(char *r, int size)
{
//	int  i;
	int  nsize;
//	int  sz;//, s1, s2;
	int  pos = 0,
	     xypos = 0;
	int decomp_size = 0;
//	char tmp_int[5];

//	char filename[256];

	char *decomp;
//	FILE *fp_cell_data;

	if (strcmp(cell.worldspace_name, worldspace_lc) != 0) {
		return 0;
	}

	cell.current_x = 0;
	cell.current_y = 0;

	if (myisupper(r[tes_rec_offset]) && myisupper(r[tes_rec_offset+1]) && myisupper(r[tes_rec_offset+2]) && myisupper(r[tes_rec_offset+3])) {
		decomp = (char *)calloc(size, 1); //!
		memcpy(decomp, r+tes_rec_offset, size-tes_rec_offset);
		decomp_size = size-tes_rec_offset;

	} else {
		decomp = (char *)calloc(size*300, 1);

		if (DecompressZLIBStream(r+tes_rec_offset+4, size-tes_rec_offset-4, decomp, &decomp_size) != 0) {
			fprintf(stderr, "Decoding of ZLIB compressed CELL record failed: %s\n", strerror(errno));
			return -1;
		}
	}

	/*****************************************
	 ****************************************/

	while (pos < decomp_size -1) {
		nsize = 0;
		memcpy(&nsize, decomp+pos+4, 2);

		/*****************************************************************************
		 * The XCLC section. Bytes 7-14 contains two integers that represent the (x,y)
		 * co-ordinates of this cell in the Worldspace's world grid.
		 ****************************************************************************/
		if (strncmp("XCLC", decomp + pos, 4) == 0) {
			memcpy(&cell.current_x, decomp+pos+6, 4);
			memcpy(&cell.current_y, decomp+pos+10, 4);
		} else if (strncmp("EDID", decomp + pos, 4) == 0) {
			strncpy(cell.name, decomp + pos + 6, nsize);
		}
		pos += 6 + nsize;
	}

	cleanup_list_x[cleanup_list_count] = cell.current_x;
	cleanup_list_y[cleanup_list_count++] = cell.current_y;

	free(decomp);

	return 0;
}

/*****************************************************************
** 5. Process4WRLDData(): Process a TES4 Worldspace record.
*****************************************************************
** WRLD (4 bytes) + Length (2 bytes) + EDID (name)
****************************************************************/

int Process4WRLDData(char *r, int size)
{
	int  i;
	int  nsize;
	int  pos = 0,
	     xypos = 0;

	char lod2_dir[256];

	pos += tes_rec_offset;

	/***********************************************
	 * EDID (name of the Worldspace (6+Name bytes)).
	 **********************************************/
	if (strncmp("EDID", r + pos, 4) == 0) {
		nsize = 0;
		memcpy(&nsize, r+pos+4, 2);
		pos += 6;
		strncpy_s(cell.worldspace_name, 1024, r + pos, nsize);
		pos += nsize;
		if (verbosity) printf("\nFound Worldspace: %s (FormID: %2.2X%2.2X%2.2X%2.2X)", 
			cell.worldspace_name,
			(unsigned char) (r[15]),
			(unsigned char) (r[14]),
			(unsigned char) (r[13]),
			(unsigned char) (r[12]));

		for (i = 0; cell.worldspace_name[i] != '\0'; i++) {
			cell.worldspace_name[i] = tolower(cell.worldspace_name[i]);
		} 

		if (strcmp(cell.worldspace_name, worldspace_lc) == 0) {
			if (verbosity) putchar('\n');
			memcpy(&cell.worldspace_formid, r+12, 4);
			if (opt_lod2) {
				MKDIR("Textures");
				MKDIR("Textures/LOD");
				sprintf_s(lod2_dir, 256, "Textures/LOD/%s", cell.worldspace_name);
				MKDIR(lod2_dir);
				for (i = 1; i < 5; i++) {
					sprintf_s(lod2_dir, 256, "Textures/LOD/%s/%d", cell.worldspace_name, i);
					MKDIR(lod2_dir);
				}
			}
			return 1;
		} else {
			if (verbosity) printf(" - Ignoring this Worldspace.\n");
		}
	} else {
		cell.worldspace_name[0] = '\0';
		printf("\nWarning: Could not find a name for this worldspace!!\n");
	}

	return 0;
}


/*****************************************************************
** 5. Process4LandData(): Process a TES4 LAND record.
*****************************************************************
** LAND (4 bytes) + Length (4 bytes) + X (4 bytes) + Y (4 bytes).
****************************************************************/

int Process4LANDData(char *r, int size)
{
	/*********************************************************
	 * Hopefully we have the X and & co-ords already from the 
	 * preceding CELL record.
	 ********************************************************/
	/*
	Did a lot of changes here to add the different layers, CHANGE_IF
	Too much to mark it
	*/

	int i, j, k, l, m, x, y, layer, //CHANGE_IF
	    decomp_size = 0,
//	    ltex_index,
	    nsize = 0,
	    pos = 0,
	    sz = 0,
	    texture_matched = 0, base_layer = 0;

	char c,
	     vtxt[9][34][34][4],
	     vimage[136][136][3],
	     atxt[4],
	     vclr[33][33][3],
	     tmp_land_filename[64],
	     *decomp;

	short int quad, t;

	float vtxt_opacity;

	float tex_opacity[9][34][34];
	int num_layer[34][34];
//	float my_rgb[4];
	float add_tex_opacity[136][136], add_rgb[136][136][3], tex_opacity_a, tex_opacity_b, tex_opacity_c;

	FILE *fp_land;

	for (i=0;i<34;i++) for (j=0;j<34;j++) num_layer[i][j]=0;

	if (strcmp(cell.worldspace_name, worldspace_lc) != 0) {
		return 0;
	}
	if (verbosity) printf("(%d, %d) ", cell.current_x, cell.current_y);
	fflush(stdout);

	if (cell.current_x < min_x) min_x = cell.current_x;
	if (cell.current_x > max_x) max_x = cell.current_x;

	if (cell.current_y < min_y) min_y = cell.current_y;
	if (cell.current_y > max_y) max_y = cell.current_y;




	worldspace_found = 1;

	if (myisupper(r[tes_rec_offset]) && myisupper(r[tes_rec_offset+1]) && myisupper(r[tes_rec_offset+2]) && myisupper(r[tes_rec_offset+3])) {
		decomp = (char *) calloc(size, 1);
		memcpy(decomp, r+tes_rec_offset, size-tes_rec_offset);
		decomp_size = size-tes_rec_offset;

	} else {
		decomp = (char *) calloc(size*300, 1);

		if (DecompressZLIBStream(r+tes_rec_offset+4, size-tes_rec_offset-4, decomp, &decomp_size) != 0) {
			fprintf(stderr, "Decoding of ZLIB compressed CELL record failed: %s\n", strerror(errno));
			//exit(1); //!!!!!!!!!!!!!!!!!!!!
			return -1;
		}
	}

	while (pos < decomp_size && 
	       strncmp("VCLR", decomp + pos, 4) != 0 && 
	       strncmp("BTXT", decomp + pos, 4) != 0 &&
	       strncmp("ATXT", decomp + pos, 4) != 0) {
	       memcpy(&nsize, decomp + pos + 4, 2);
		pos += 6 + nsize;
	}

#ifdef FROMTES4LL
	if (opt_read_heightmap) {
		pos = 3289-6;
		if (pos < decomp_size && strncmp("VHGT", decomp + pos, 4) == 0) {
			float start_height;
			memcpy(&start_height, decomp + pos + 6, 4);
			//cout << start_height << ":";
			for (y = 0; y < 33; y++) {
				start_height += decomp[pos + 6 + 4 + y*33];
				float running_height = start_height;
				for (x = 1; x < 33; x++) {
					running_height += decomp[pos+6+x+4+y*33];
					if (heightmap) {
						heightmap->SetElement((cell.current_x * 32 + x - x_cell *32 - 1) * npoints, 
							(cell.current_y * 32 + y - y_cell *32 - 1) * npoints, running_height);
					}
				}
			}
		}
	}
#endif

	if (!opt_lod_tex) {
		free(decomp);
		return 0;
	}

	/***********************************
	 ** Write out Normals to a BMP file.
	 **********************************/
	if (opt_normals) {

		sprintf_s(tmp_land_filename, 64, "%s/normal.%d.%d.bmp", TMP_NORMAL_DIR, cell.current_x, cell.current_y);

		if ((fp_land = fopen(tmp_land_filename, "wb")) == 0) {
			fprintf(stderr, "Unable to create a temporary normals file for writing, %s: %s\n", 
				tmp_land_filename, strerror(errno));
			exit(1);
		}

		WriteBMPHeader(fp_land, 32, 32, 24);
		for (i = 1; i < 33; i++) {
			for (j = 0; j < 96; j++) {
				c = (unsigned char) (decomp[10+6+(99*i)+2+j] + 125);
				fputc(c, fp_land);
			}
		}

		fclose(fp_land);
	}


	pos = 3289+1096;
	memset(vclr, 255, 3267);
	if (pos < decomp_size && strncmp("VCLR", decomp + pos, 4) == 0) {
		for (i = 0; i < 3267; i++) {
			if (pos+6+i < decomp_size) {
				memcpy((&vclr[0][0][0])+i, decomp+pos+6+i, 1);
			}
		}
		pos += 3267 + 6;
	}

	/**********************************************************
	 ** TEXTURE MATCHING:
	 ** Now accumulate the VTXT data in to a 34x34 array, storing the FormID for each
	 ** texture for each grid point (each FormID is 4 bytes).
	 *********************************************************/

	memset(&tex_opacity, 0, sizeof(tex_opacity));

	memset(vtxt[0], 0, sizeof(vtxt));
	for (i = 0; i < 34; i++) {
		for (j = 0; j < 34; j++) {
			tex_opacity[0][i][j] = 1.0f;
			for (k = 0; k < 8; k++) tex_opacity[k+1][i][j] = 0.f; //CHANGE_IF
		}
	}

	while (pos < decomp_size) {
		nsize = (unsigned char) decomp[pos + 4] + 256 * ((unsigned char) decomp[pos+5]);

		if (strncmp("ATXT", decomp+pos, 4) == 0) {
			memcpy(atxt, decomp+pos+6, 4);
			quad = decomp[pos+6+4];
			layer = decomp[pos+6+6]; //Get layer number, IF
			if (layer > 7) {
				printf("Found strange layer number %i\n",layer);
				layer = 7;
			}
		} else if (strncmp("BTXT", decomp+pos, 4) == 0) {
			memcpy(atxt, decomp+pos+6, 4);
			quad = decomp[pos+6+4];

			if (quad == 0) {
				for (i = 0; i < 17 ; i++) {
					for (j = 0; j < 17; j++) {
						if (vtxt[0][i][j][0] == 0 && vtxt[0][i][j][1] == 0 && vtxt[0][i][j][2] == 0 && vtxt[0][i][j][3] == 0) {
							memcpy(vtxt[0][i][j], atxt, 4);
							tex_opacity[0][i][j] = 1.0;
						} 
					}
				}
			} else if (quad == 1) {
				for (i = 0; i < 17; i++) {
					for (j = 17; j < 34; j++) {
						if (vtxt[0][i][j][0] == 0 && vtxt[0][i][j][1] == 0 && vtxt[0][i][j][2] == 0 && vtxt[0][i][j][3] == 0) {
							memcpy(vtxt[0][i][j], atxt, 4);
							tex_opacity[0][i][j] = 1.0;
						}
					}
				}
			} else if (quad == 2) {
				for (i = 17; i < 34 ; i++) {
					for (j = 0; j < 17; j++) {
						if (vtxt[0][i][j][0] == 0 && vtxt[0][i][j][1] == 0 && vtxt[0][i][j][2] == 0 && vtxt[0][i][j][3] == 0) {
							memcpy(vtxt[0][i][j], atxt, 4);
							tex_opacity[0][i][j] = 1.0;
						}
					}
				}
			} else if (quad == 3) {
				for (i = 17; i < 34; i++) {
					for (j = 17; j < 34; j++) {
						if (vtxt[0][i][j][0] == 0 && vtxt[0][i][j][1] == 0 && vtxt[0][i][j][2] == 0 && vtxt[0][i][j][3] == 0) {
							memcpy(vtxt[0][i][j], atxt, 4);
							tex_opacity[0][i][j] = 1.0;
						}
					}
				}
			}
		} else if (strncmp("VTXT", decomp+pos, 4) == 0) {

			for (k = 6; k < (6+nsize); k += 8) {
				memcpy(&t, decomp+pos+k, 2); 
				//= (unsigned int) ((unsigned char) decomp[pos+k] + (unsigned char) decomp[pos+k+1]*256);
				i = (int) t/17;
				j = t - i*17;

				if (quad == 1) {
					j += 17;
				} else if (quad == 2) {
					i += 17;
				} else if (quad == 3) {
					i += 17; j+= 17;
				}

				memcpy(&vtxt_opacity, decomp+pos+k+4, 4);
				memcpy(vtxt[layer+1][i][j], atxt, 4);
				tex_opacity[layer+1][i][j] = vtxt_opacity;
				num_layer[i][j] = layer + 1;

			}
		}

		pos += 6 + nsize;
	}

	/* Create an RGB image in memory. The TES4 texture array is 34x34. We discard the 'frame', leaving 
         * a 32x32 image. Looks fine in game. What the precise 'correct' behaviour should be, I don't know.
         *
         * Compare the FormID of the VTXT position with all those LTEX records we have stored.
	 * If the 4th byte of both is 0, it's a modindex 00 texture (from the master). If both are non-zero
         * then they're plugins and changeable; in which case just the first 3 bytes can be compared.
         */
	
	memset(vimage, 0, sizeof(vimage));


	for (i = 1; i < 33; i++) {
		for (j = 1; j < 33; j++) {
			for (y = 0; y < opt_q; y++) { //Clean additional layers
					for (x = 0; x < opt_q; x++) {
						add_tex_opacity[y+opt_q*i][x+opt_q*j] = 0.f;
						for (m = 0; m < 3; m++) {
							add_rgb[y+opt_q*i][x+opt_q*j][m] = 0;
						}
					}
				}
			for (l=0; l<=num_layer[i][j]; l++) {
				texture_matched = 0;
				
				for (k = 0; k < lod_ltex.count; k++) {			
					if (memcmp(&vtxt[l][i][j], &lod_ltex.formid[k], 4) == 0) {
						texture_matched = 1;
					} else if (vtxt[l][i][j][3] != 0 && lod_ltex.formid[k] >= 16777216 && memcmp(&vtxt[l][i][j], &lod_ltex.formid[k], 3) == 0) {
						texture_matched = 1;
					} 

					if (texture_matched) {
						for (y = 0; y < opt_q; y++) {
							for (x = 0; x < opt_q; x++) {
								if (l==0) {
									memcpy(&vimage[y+opt_q*i][x+opt_q*j], &lod_ltex.rgb[k][y][x], 3);				
									add_tex_opacity[y+opt_q*i][x+opt_q*j] = tex_opacity[0][i][j];	
									for (m = 0; m < 3; m++) {
										add_rgb[y+opt_q*i][x+opt_q*j][m] = (float) (unsigned char) lod_ltex.rgb[k][y][x][m];
									}
									base_layer=opt_q*opt_q;
								} else if (tex_opacity[l][i][j] && (base_layer)) {
									for (m = 0; m < 3; m++) {
										add_tex_opacity[y+opt_q*i][x+opt_q*j] = tex_opacity[l][i][j];	
										add_rgb[y+opt_q*i][x+opt_q*j][m] = (float) (unsigned char) lod_ltex.rgb[k][y][x][m];
									}
									base_layer--;
								} else if (tex_opacity[l][i][j] && (1)) {
									if (opt_blending == 0) { //use Lightwaves "best win" method
										if (tex_opacity[l][i][j] > add_tex_opacity[y+opt_q*i][x+opt_q*j]) {
											for (m = 0; m < 3; m++) {
												add_tex_opacity[y+opt_q*i][x+opt_q*j] = tex_opacity[l][i][j];	
												add_rgb[y+opt_q*i][x+opt_q*j][m] = (float) (unsigned char) lod_ltex.rgb[k][y][x][m];
											}
										}
									} else if (1) {
										tex_opacity_b = (tex_opacity[l][i][j]);	
										tex_opacity_a = add_tex_opacity[y+opt_q*i][x+opt_q*j];
										tex_opacity_c = tex_opacity_a + (1.0f - tex_opacity_a)*tex_opacity_b;
										add_tex_opacity[y+opt_q*i][x+opt_q*j] = tex_opacity_c;
										for (m = 0; m < 3; m++) {
											add_rgb[y+opt_q*i][x+opt_q*j][m] = (1.f/tex_opacity_c) * (
												tex_opacity_a*add_rgb[y+opt_q*i][x+opt_q*j][m] + (1.f - tex_opacity_a)*
												tex_opacity_b*((float) (unsigned char) lod_ltex.rgb[k][y][x][m])
												);
										}
									} else if (0) { //Only for testing, ignored for the time being, IF
										for (m = 0; m < 3; m++) {
											tex_opacity_c = add_rgb[y+opt_q*i][x+opt_q*j][m];
											add_rgb[y+opt_q*i][x+opt_q*j][m] = tex_opacity[l][i][j]*((float) (unsigned char) lod_ltex.rgb[k][y][x][m])
												+ (1.0 - tex_opacity[l][i][j])*tex_opacity_c;
										}
									}
								}
							}
						}
						k = lod_ltex.count;
					}
				}
			}
			for (y = 0; y < opt_q; y++) {
				for (x = 0; x < opt_q; x++) {
					for (m = 0; m < 3; m++) {
						if (opt_blending) {
							vimage[y+opt_q*i][x+opt_q*j][m] = 
							(unsigned char) (((float) (unsigned char) vimage[y+opt_q*i][x+opt_q*j][m]) * (1.f - add_tex_opacity[y+opt_q*i][x+opt_q*j]) 
							+ (add_rgb[y+opt_q*i][x+opt_q*j][m]) * add_tex_opacity[y+opt_q*i][x+opt_q*j]);
						} else {
							if (add_tex_opacity[y+opt_q*i][x+opt_q*j] > 0.5)
								vimage[y+opt_q*i][x+opt_q*j][m] = (unsigned char) add_rgb[y+opt_q*i][x+opt_q*j][m];
						}
						//vimage[y+opt_q*i][x+opt_q*j][m] = (unsigned char) add_rgb[y+opt_q*i][x+opt_q*j][m];
						if (!opt_no_vclr) vimage[y+opt_q*i][x+opt_q*j][m] = (unsigned char) ((float) (unsigned char) vimage[y+opt_q*i][x+opt_q*j][m] * ((float) (unsigned char) vclr[i][j][2-m]/ 255.0f));									
					}
				}
			}

/*
			if (!texture_matched) {
				{	
					char l[4];
					memcpy(l, &lod_ltex.formid[k], 4);
					printf("No match  for texture %2.2X %2.2X %2.2X %2.2X\n", (unsigned char) vtxt[i][j][0], (unsigned char) vtxt[i][j][1], (unsigned char) vtxt[i][j][2], (unsigned char) vtxt[i][j][3]);
					printf("To match with texture %2.2X %2.2X %2.2X %2.2X!\n", (unsigned char) l[0], (unsigned char) l[1], (unsigned char) l[2], (unsigned char) l[3]);
				}
			}
*/


		}
	}

	DumpCellBMP(cell.current_x, cell.current_y, vimage);

	// fclose(fp_land);

	free(decomp);

	total_land_copied++;

	return 0;
}

int DumpCellBMP(int cx, int cy, char vimage[136][136][3])
{
	int i, j; //, r;
	int l = 0;

	char filename[64];

	FILE *fp_c;

	sprintf_s(filename, 64, "%s/partial.%d.%d.bmp", TMP_TEX_DIR, cx, cy);

	if ((fp_c = fopen(filename, "wb")) == 0) {
               fprintf(stderr, "Unable to create a cell BMP called %s: %s\n",
                        filename, strerror(errno));
		exit(1);
	}

	WriteBMPHeader(fp_c, opt_q*32, opt_q*32, 24); // 24-bit 32x32 (or more, if opt_q is greater) image.

	l = (32 * opt_q) + opt_q; // The right and top-most limit of the texture image to copy.

	for (i = opt_q; i < l; i++) {
		for (j = opt_q; j < l; j++) {
			fwrite(vimage[i][j], 3, 1, fp_c);
		}
	}

	fclose(fp_c);

	if (opt_lod2) {
		sprintf_s(filename, 64, "partial.%d.%d.bmp", cx, cy);
		LOD2_Partial(filename, cx, cy, TEXTURES);
		if (opt_normals) {
			sprintf_s(filename, 64, "normal.%d.%d.bmp", cx, cy);
			LOD2_Partial(filename, cx, cy, NORMALS);
		}
	}

	return 0;
}

int LOD2_Partial(char *filename, int cx, int cy, int mode)
{
	char lod_filename[256];
	char dds_filename[256];
	char dds_command[256];
//	char lod_command[256];

//	FILE *fp_p;

//	sprintf(dds_command, "%s /M /3 %s", DDS_CONVERTOR, filename);
	if (mode == TEXTURES) {
		sprintf_s(dds_command, 256, "copy %s\\%s %s", TMP_TEX_DIR, filename, filename);
	} else {
		sprintf_s(dds_command, 256, "copy %s\\%s %s", TMP_NORMAL_DIR, filename, filename);
	}
	system(dds_command);
	sprintf_s(dds_command, 256, "%s %s", DDS_CONVERTOR, filename);
	printf("Running External DDS Convertor: %s\n", dds_command);
	printf("Running %s\n", dds_command);
	system(dds_command);

	strcpy_s(dds_filename, 256, filename);
	memcpy(dds_filename + strlen(dds_filename) - 3, "dds", 3);


       // Windoze / DOS

	//sprintf(lod_filename, "%d.%.2d.%.2d.32.dds", world_index, cx, cy);
	//printf("LOD2_Partial\n");
	if (cx >= 0 && cy >= 0) {
		sprintf_s(lod_filename, 256, "textures\\lod2\\%s\\1", cell.worldspace_name);
		MKDIR(lod_filename);
		if (mode == TEXTURES) {
			sprintf_s(lod_filename, 256, "textures\\lod2\\%s\\1\\%d.%d.dds", cell.worldspace_name, cx, cy);
		} else {
			sprintf_s(lod_filename, 256, "textures\\lod2\\%s\\1\\%d.%d_fn.dds", cell.worldspace_name, cx, cy);
		}
	} else if (cx >= 0 && cy <= 0) {
		sprintf_s(lod_filename, 256, "textures\\lod2\\%s\\2", cell.worldspace_name);
		MKDIR(lod_filename);
		if (mode == TEXTURES) {
			sprintf_s(lod_filename, 256, "textures\\lod2\\%s\\2\\%d.%d.dds", cell.worldspace_name, cx, cy);
		} else {
			sprintf_s(lod_filename, 256, "textures\\lod2\\%s\\2\\%d.%d_fn.dds", cell.worldspace_name, cx, cy);
		}
	} else if (cx <= 0 && cy >= 0) {
		sprintf_s(lod_filename, 256, "textures\\lod2\\%s\\3", cell.worldspace_name);
		MKDIR(lod_filename);
		if (mode == TEXTURES) {
			sprintf_s(lod_filename, 256, "textures\\lod2\\%s\\3\\%d.%d.dds", cell.worldspace_name, cx, cy);
		} else {
			sprintf_s(lod_filename, 256, "textures\\lod2\\%s\\3\\%d.%d_fn.dds", cell.worldspace_name, cx, cy);
		}
	} else {
		sprintf_s(lod_filename, 256, "textures\\lod2\\%s\\4", cell.worldspace_name);
		MKDIR(lod_filename);
		if (mode == TEXTURES) {
			sprintf_s(lod_filename, 256, "textures\\lod2\\%s\\4\\%d.%d.dds", cell.worldspace_name, cx, cy);
		} else {
			sprintf_s(lod_filename, 256, "textures\\lod2\\%s\\4\\%d.%d_fn.dds", cell.worldspace_name, cx, cy);
		}
	}

	if (!opt_no_move) {
		sprintf_s(dds_command, 256, "move %s %s", dds_filename, lod_filename);
		printf("Doing %s\n", dds_command);
		system(dds_command);
		if (_unlink(dds_filename) == -1)
			fprintf(stdout, "Could not delete dds %s\n",dds_filename);
	}

	return 0;
}

int Process4REFRData(char *r, int size)
{
//	int  i;
	int  pos = 0,
	     xypos = 0;
	int base_formid;
	float scale = 1.0;

	unsigned short int  nsize;

	char xdat[24];
	char tmp_vwd_filename[256];

	FILE *fp;

	if (strcmp(cell.worldspace_name, worldspace_lc) != 0) {
		return 0;
	}

	if (!(r[9] & 0x80) && !opt_vwd_everything) {
		return 0;
	}

	total_refr++;
	putchar('V');

	pos += tes_rec_offset;

	/***********************************************************
	 * NAME (Base FormID) or DATA (X,Y,Z) of this Placed Object.
	 **********************************************************/
	while (pos < size) {
		memcpy(&nsize, r + pos + 4, 2);
		if (strncmp("NAME", r + pos, 4) == 0) {
			memcpy(&base_formid, r + pos + 6, 4);
		} else if (strncmp("DATA", r + pos, 4) == 0) {
			memcpy(xdat, r + pos + 6, 24);
		} else if (strncmp("XSCL", r + pos, 4) == 0) {
			memcpy(&scale, r + pos + 6, 4);
		}
		pos += 6 + nsize;
	}
	scale *= 100.0f;

//	printf("Found VWD: %d\n", base_formid);
	total_vwd++;

	sprintf_s(tmp_vwd_filename, 256, "%s/%s.%d.%d.tmp", TMP_VWD_DIR, worldspace_lc, cell.current_x, cell.current_y);
	if ((fp = fopen(tmp_vwd_filename, "ab+")) == 0) {
		fprintf(stderr, "Unable to create temporary VWD file called %s: %s\n",
			tmp_vwd_filename, strerror(errno));
		CleanUp();
		exit(1);
	}

	fwrite(&base_formid, 4, 1, fp);
	fwrite(xdat, 24, 1, fp);
	fwrite(&scale, 4, 1, fp);

	fclose(fp);

	return 0;
}

void Log(char *info)
{
	FILE *fp;

	return; // Ignore function.

	if ((fp = fopen("log_file", "a")) == 0) {
		fprintf(stderr, "Unable to open log file for appending "
			"log data (%s): %s\n",
			"log_file", strerror(errno));
		return;
	}

	fprintf(fp, info);
	fflush(fp);

	fclose(fp);
}

int DecompressZLIBStream(char *input, int input_size, char output[], int *output_size)
{
	z_stream z;

//	int count;
	int status;
	int avail_out = input_size*60;

	z.zalloc = Z_NULL;
	z.zfree = Z_NULL;
	z.opaque = Z_NULL;

	if (inflateInit(&z) != Z_OK) {
		fprintf(stderr, "inflateInit failed: %s\n", strerror(errno));
	}

	z.avail_in = input_size;
	z.next_in = (Bytef*)input;

	z.avail_out = avail_out;
	z.next_out = (Bytef*)output;

       	status = inflate( &z, Z_NO_FLUSH );
	if (status == Z_STREAM_ERROR) {
		return -1;
	}

	*output_size += avail_out - z.avail_out;

	(void)inflateEnd(&z);

	return 0;
}


int CompressZLIBStream(char *input, int input_size, char output[], int *output_size, int compress_level)
{
	z_stream z;

//	int count;
	int status;
	int avail_out = input_size;

	z.zalloc = Z_NULL;
	z.zfree = Z_NULL;
	z.opaque = Z_NULL;

	if (deflateInit(&z, compress_level) != Z_OK) {
		fprintf(stderr, "deflateInit failed: %s\n", strerror(errno));
		return -1;
	}

	z.avail_in = input_size;
	z.next_in = (Bytef*) input;
	z.avail_out = avail_out;
	z.next_out  = (Bytef*) output;

       	status = deflate( &z, 1);

	*output_size += avail_out - z.avail_out;

	(void)deflateEnd(&z);

	return 0;
}

int CleanUp()
{
	int i;

	char filename[128];

	for (i = 0; i < cleanup_list_count; i++) {
		if (opt_lod_tex) {
			sprintf_s(filename, 128, "%s/partial.%d.%d.bmp", TMP_TEX_DIR, cleanup_list_x[i], cleanup_list_y[i]);
			_unlink(filename);
				
		}
		if (opt_vwd) {
			sprintf_s(filename, 128, "%s/%s.%d.%d.tmp", TMP_VWD_DIR, worldspace_lc, cleanup_list_x[i], cleanup_list_y[i]);
			_unlink(filename);
				
		}
		if (opt_normals) {
			sprintf_s(filename, 128, "%s/normal.%d.%d.tmp", 
				TMP_NORMAL_DIR, cleanup_list_x[i], cleanup_list_y[i]);
			_unlink(filename);
				
		}
	}

	if (opt_vwd) {
		_rmdir(TMP_VWD_DIR);
	}

	if (opt_lod2) {
		_rmdir(TMP_NORMAL_DIR);
	}

	_rmdir(TMP_TEX_DIR);
	
	return 0;
}

int CleanUpDir(char *dirname)
{
	DIR             *dis;
	struct dirent   *dir;

	char filename[512];

	if ((dis = opendir(dirname)) != NULL) {
		if (verbosity) printf("Cleaning up temporary directory (%s) ... ", dirname);
		fflush(stdout);

		while ((dir = readdir(dis)) != NULL) {
			if (dir->d_name[0] != '.' && (dir->d_name[1] != '\0' || (dir->d_name[1] != '.' && dir->d_name[2] != '\0'))) {
				sprintf_s(filename, 512, "%s/%s", dirname, dir->d_name);
				if(_unlink(filename) == -1 )
					fprintf(stdout,  "Could not delete partial %s, errno=%i\n" , filename,errno);
			}
		}
		closedir(dis);
		if (verbosity) printf("finished.\n");
		fflush(stdout);
	}
	_unlink(dirname);

	return 0;
}

void WriteBMPHeader(FILE *fp_out, int sx, int sy, int bpp)
{
	int i;

	char bmp_head[54] = {
			0x42, 0x4D, 0x98, 0xEA, 0x00, 0x00, 0x00, 0x00, 
			0x00, 0x00, 0x36, 0x00, 0x00, 0x00, 0x28, 0x00, 
			0x00, 0x00, 0xC8, 0x00, 0x00, 0x00, 0x64, 0x00, 
			0x00, 0x00, 0x01, 0x00, 0x20, 0x00, 0x00, 0x00, 
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x12, 0x0B, 
			0x00, 0x00, 0x12, 0x0B, 0x00, 0x00, 0x00, 0x00, 
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00
	};

	i = (sx*sy*4)+54;

	memcpy(bmp_head+2, &i, 4);
	memcpy(bmp_head+18, &sx, 4);
	memcpy(bmp_head+22, &sy, 4);
	//memcpy(bmp_head+28, &bpp, 1);  // Correct the header for bits per pixel.
	bmp_head[28] = bpp;

	i = (sx*sy*4);
	memcpy(bmp_head+34, &i, 4);

	for (i = 0; i < 54; i++) {
		fputc(bmp_head[i], fp_out);
	}
}

int HumptyLODs(void)
{
	int i, j, l, quad;
	int dsize = 32;
	int quad_max = 32,
	    quad_start = 4;
	int lod_min_x, lod_max_x,
	    lod_min_y, lod_max_y;
	int world_index;
	char dds_command[128];
	char lod_bmpname[64];
	char lod_ddsname[64];
	char dest_dir[256];
	char full_lod_map[128];

	world_index = cell.worldspace_formid + (opt_load_index * 16777216);

//	printf("Worldspace formid is %d\n", cell.worldspace_formid);

	lod_min_x = 32 * (int) ((min_x - 31)/32);
	lod_max_x = 32 * (int) (max_x / 32);
	lod_min_y = 32 * (int) ((min_y - 31)/32);
	lod_max_y = 32 * (int) (max_y / 32);

	if (opt_debug) {
		printf("\n\n");
		printf("Cell Minimum and Maximum Limits:\n"
		       "--------------------------------\n\n");
		printf("Max X is %d, Min X is %d\n", max_x, min_x); 
		printf("Max Y is %d, Min Y is %d\n", max_y, min_y);
		printf("LOD X: from %d to %d\n", lod_min_x, lod_max_x);
		printf("LOD Y: from %d to %d\n", lod_min_y, lod_max_y);
		printf("--------------------------------\n\n");
	}
	printf("\n\n");

	if (opt_tes_mode == TES_OBLIVION) {  // Oblivion's quads were only 32x32 cell quads.
		quad_start = 32; 
	} else {
		quad_start = 4;              // Fallout3 / FalloutNV/Skyrim use 4x4, 8x8, 16x16 and 32x32 cells quads.
	}

	/****************************************************************************************
	 * OBLIVION: WRLD_FORMID.32.16.32.bmp  (e.g. 60.32.16.32.bmp)
      * FALLOUT3 & NV: textures/lod/WORLDSPACE/diffuse/WORLDSPACE.n.level[4,8,16,32].x-12.y24.dds
	 * SKYRIM:  textures/landscape/terrain/WORLDSPACE.[4,8,16,32].-12.24.dds
	 ***************************************************************************************/

	if (!opt_lod2 || opt_full_map) {
 	    for (quad = quad_start; quad <= quad_max; quad = quad * 2) {
			for (i = lod_min_y; i <= lod_max_y; i+=quad) {
				for (j = lod_min_x; j <= lod_max_x; j+=quad) {
					/*********************************
					 ** Generate LOD Quads of Normals.
					 ********************************/
					if (opt_normals) {

						if (opt_tes_mode == TES_OBLIVION) {
							sprintf_s(lod_bmpname, 64, "%d.%.2d.%.2d.%.2d_fn.bmp", world_index, j, i, quad);
						} else if (opt_tes_mode == TES_FALLOUT3 || opt_tes_mode == TES_FALLOUTNV) {
							sprintf_s(lod_bmpname, 64, "%s.n.level%d.x%d.y%d.bmp", worldspace_lc, quad, j, i);
						} else {
							sprintf_s(lod_bmpname, 64, "%s.%d.%d.%d_n.bmp", worldspace_lc, quad, j, i);
						}
						HumptyLOD(lod_bmpname, TMP_NORMAL_DIR, "normal", j, j+quad-1, i, i+quad-1, dsize, 1, 1, NORMALS);

						if (!opt_no_dds) {
							sprintf_s(dds_command, 128, "%s %s", DDS_CONVERTOR, lod_bmpname);
							printf("Running External DDS Convertor on Normmal: %s\n", dds_command);
							system(dds_command);

							if (opt_tes_mode == TES_OBLIVION) {
								sprintf_s(lod_ddsname, 64, "%d.%.2d.%.2d.%.2d_fn.dds", world_index, j, i, quad);
								sprintf_s(dest_dir, 256, LOD_OUTPUT_DIR_OBLIVION_DOS);
							} else if (opt_tes_mode == TES_FALLOUT3 || opt_tes_mode == TES_FALLOUTNV) {
								sprintf_s(dest_dir, 256, LOD_OUTPUT_DIR_FALLOUT3_DOS, worldspace_lc);
								strcat_s(dest_dir, 256, "\\normals");
								sprintf_s(lod_ddsname, 64,  "%s.n.level%d.x%d.y%d.dds", worldspace_lc, quad, j, i);
							} else {
								sprintf_s(dest_dir, 256, LOD_OUTPUT_DIR_SKYRIM_DOS, worldspace_lc);
								sprintf_s(lod_ddsname, 64, "%s.%d.%d.%d_n.dds", worldspace_lc, quad, j, i);
							}

							if (!opt_no_move) {
								sprintf_s(dds_command, 128, "move %s %s\\%s", lod_ddsname, dest_dir, lod_ddsname);
								printf("Moving Normal with %s\n", dds_command);
								system(dds_command);
							}
						}

						if (!opt_debug && !opt_no_dds) {
							if (_unlink(lod_bmpname) == -1)
								fprintf(stdout, "Could not delete normal bmp %s\n",lod_bmpname);
						}
					}

					/***************************************
					 ** Generate LOD Quads of Land Textures.
					 **************************************/

					if (opt_tes_mode == TES_OBLIVION) {
						sprintf_s(lod_bmpname, 64, "%d.%.2d.%.2d.%.2d.bmp", world_index, j, i, quad);
					} else if (opt_tes_mode == TES_FALLOUT3 || opt_tes_mode == TES_FALLOUTNV) {
						sprintf_s(lod_bmpname, 64, "%s.n.level%d.x%d.y%d.bmp", worldspace_lc, quad, j, i);
					} else {
						sprintf_s(lod_bmpname, 64, "%s.%d.%d.%d.bmp", worldspace_lc, quad, j, i);
					}

					//CHANGE_IF
					//Skyrim textures seem to be up-down flipped
					if (!opt_no_colorlods) {
						if (opt_tes_mode == TES_SKYRIM) 
							HumptyLOD(lod_bmpname, TMP_TEX_DIR, "partial", j, j+quad-1, i, i+quad-1, dsize, 0, opt_q, TEXTURES);
						else
							HumptyLOD(lod_bmpname, TMP_TEX_DIR, "partial", j, j+quad-1, i, i+quad-1, dsize, 1, opt_q, TEXTURES);
					}

					if (!opt_no_dds && !opt_no_colorlods) {
						// Windoze / DOS

						sprintf_s(dds_command, 128, "%s %s", DDS_CONVERTOR, lod_bmpname);
						printf("Running External DDS Convertor on Texture: %s\n", dds_command);
						system(dds_command);

						if (opt_tes_mode == TES_OBLIVION) {
							sprintf_s(lod_ddsname, 64, "%d.%.2d.%.2d.%.2d.dds", world_index, j, i, quad);
							sprintf_s(dest_dir, 256, LOD_OUTPUT_DIR_OBLIVION_DOS);
						} else if (opt_tes_mode == TES_FALLOUT3 || opt_tes_mode == TES_FALLOUTNV) {
							sprintf_s(dest_dir, 256, LOD_OUTPUT_DIR_FALLOUT3_DOS, worldspace_lc);
							sprintf_s(lod_ddsname, 64, "%s.n.level%d.x%d.y%d.dds", worldspace_lc, quad, j, i);
							strcat_s(dest_dir, 256, "\\diffuse");
						} else {
							sprintf_s(dest_dir, 256, LOD_OUTPUT_DIR_SKYRIM_DOS, worldspace_lc);
							sprintf_s(lod_ddsname, 64, "%s.%d.%d.%d.dds", worldspace_lc, quad, j, i);
						}

						if (!opt_no_move) {
							sprintf_s(dds_command, 128, "move %s %s\\%s", lod_ddsname, dest_dir, lod_ddsname);
							printf("Moving Texture with %s\n", dds_command);
							system(dds_command);
						}

						// Unix
						/*
						sprintf(dds_command, "mv %s %s/%s", lod_ddsname, dest_dir, lod_ddsname);
						system(dds_command);
						*/

						if (!opt_debug && !opt_no_dds) {
							if (_unlink(lod_bmpname) == -1)
								fprintf(stdout, "Could not delete lof bmp %s\n",lod_bmpname);
						}
	
					}
				}
			}
		}
	}

	if (opt_full_map) {
		char filename[1000];
		char filename1[1000];
		char filename2[1000];
		if (opt_install_dir)
			sprintf_s(filename2,1000,"%s\\%sMap.dds",opt_install_dir,worldspace);
		sprintf_s(filename,1000,"%sMap.bmp",worldspace);
		sprintf_s(filename1,1000,"%sMap.dds",worldspace);
		remove (filename);		
		
		printf("Generating a complete map of the terrain called %s\n", filename2);
		HumptyLOD(filename, TMP_TEX_DIR, "partial", min_x, max_x, min_y, max_y, dsize, opt_flip, opt_q, TEXTURES);
		sprintf_s(dds_command, 128, "%s %s", DDS_CONVERTOR, filename);
		if (!opt_no_dds) {
			remove (filename1);
			printf("Running External DDS Convertor on Texture: %s\n", dds_command);
			system(dds_command);
		}
		
		if (opt_install_dir) {
			remove (filename2);
			if ((rename (filename1, filename2)) != 0) {
				printf("Could not move %s to %s",filename1,filename2);
			}						
		}
	}

	if (opt_normals && opt_full_map) {
		sprintf_s(full_lod_map, 128, FULL_LOD_NORMAL_MAP, worldspace);
		HumptyLOD(full_lod_map, TMP_NORMAL_DIR, "normal", min_x, max_x, min_y, max_y, dsize, 0, 1, NORMALS);
	}
	return 0; //CHANGE_IF
}

int HumptyLOD(char *lod_filename, char *tmp_bmp_dir, char *fprefix, int lmin_x, int lmax_x, int lmin_y, int lmax_y, int dsize, int invert, int qual, int mode)
{
	int i, j, k, m;
	int c;

	int default_colour;

	int x, y;
	int x_range, y_range;
//	int tx ;//, ty;
//	int cy;
//	int sz, s1, s2; //, s3;
	int normal_rgb = 0x007D7DF8; //125 + (256*125) + (256*248); // 0x7D7DF800; // Default flat plain normals (blue).
	int quad_found = 0;
//	int global_height_offset;
	int col_offset = 0,
	    row_sum = 0;
	int m1 = 0,
	    m2 = 0;
//	int t;
	short int sint = 0;
	int lint = 0;

	int lod_x_min = 0, lod_y_min = 0,
	    lod_x_max = 0, lod_y_max = 0;

	char tmp_int[5];

//	char cell_name[256];
	char tex_data[32000];

//	char land_data[32000];
//	char vnml_data[16384];
//	char vhgt_data[16384];

	char //cell_filename[64],
	     land_filename[64];
	     
	FILE *fp_o,
//	     *fp_cell,
	     *fp_land;

	// Recalculate min and max x and y values.

	if ((fp_o = fopen(lod_filename, "wb")) == 0) { //CHANGE_IF (fixed typo)
		fprintf(stderr, "Cannot create a new exported bmp file (%s): %s\n",
			lod_filename, strerror(errno));
		exit(1);
	}

	x_range = lmax_x-lmin_x+1;
	y_range = lmax_y-lmin_y+1;

	printf("Generating new BMP output file called: %s (Size %dx%d)\n", lod_filename, x_range*dsize*qual, y_range*dsize*qual);

	WriteBMPHeader(fp_o, dsize*qual*x_range, dsize*qual*y_range, 24);

	/****************************************************************
	 ** Look for the default texture (FormID equivalent 00000000)
	 ** Then fill initially fill the image with this texture colour.
	 ***************************************************************/

	default_colour = 0;
	for (i = 0; i < lod_ltex.count; i++) {
		if (lod_ltex.formid[i] == 0) {
			memcpy(&default_colour, lod_ltex.rgb[i], 3);
			break;
		}
	}

	fseek(fp_o, 54, SEEK_SET);
		
	for (y = 0; y < y_range; y++) {
		for (x = 0; x < x_range; x++) {
			if (mode == TEXTURES) {
				sprintf_s(land_filename, 64, "%s/partial.%d.%d.bmp", tmp_bmp_dir, x+lmin_x, y+lmin_y);
			} else {
				sprintf_s(land_filename, 64, "%s/normal.%d.%d.bmp", tmp_bmp_dir, x+lmin_x, y+lmin_y);
			}
			
			if ((fp_land = fopen(land_filename, "rb")) == 0) {
				for (k = 0; k < dsize; k++) {
					for (i = 0; i < qual; i++) {
						if (!invert) {
							fseek(fp_o, 54+(y*qual*qual*dsize*dsize*x_range*3) + (3*x*qual*dsize) + (3*i*qual*x_range*dsize) + (k*qual*3*qual*x_range*dsize), SEEK_SET);
						} else {
							fseek(fp_o, 54+((y_range-y-1)*qual*qual*dsize*dsize*x_range*3) + (3*x*qual*dsize) + (3*(qual-i-1)*qual*x_range*dsize) + ((dsize-k-1)*qual*3*qual*x_range*dsize), SEEK_SET);
						}
						for (m = 0; m < dsize; m++) {
							for (j = 0; j < qual; j++) {
								if (mode == NORMALS) {
									fwrite(&normal_rgb, 3, 1, fp_o);
								} else {
									fwrite(&lod_ltex.rgb[0][i][j], 3, 1, fp_o);
								}
							}
						}
					}
				}
				continue;
			} else {
				fseek(fp_land, 54, SEEK_SET);
				if (!invert) {
					for (k = 0; k < dsize; k++) {
						for (i = 0; i < qual; i++) {
							fseek(fp_o, 54+(y*qual*qual*dsize*dsize*x_range*3) + (3*x*qual*dsize) + (3*i*qual*x_range*dsize) + (k*qual*3*qual*x_range*dsize), SEEK_SET);
							fread(tex_data, 3*dsize*qual, 1, fp_land);
							fwrite(tex_data, 3*dsize*qual, 1, fp_o);
						}
					}
				} else {
					for (k = 0; k < dsize; k++) {
						for (i = (qual) - 1; i >= 0; i--) {
							fseek(fp_o, 54+((y_range-y-1)*qual*qual*dsize*dsize*x_range*3) + (3*x*qual*dsize) + (3*(qual-i-1)*qual*x_range*dsize) + ((dsize-k-1)*qual*3*qual*x_range*dsize), SEEK_SET);
							fread(tex_data, 3*dsize*qual, 1, fp_land);
							fwrite(tex_data, 3*dsize*qual, 1, fp_o);
						}
					}
				}
					
				fclose(fp_land);
			}
		}
	}

	fclose(fp_o);

	return 0;
}

int ReadLODTextures(char *filename)
{
	int i = 0,
	    j = 0,
	    k = 0,
	    p = 0,
	    t = 0;

	char s[1024],
	     fname[128],
	     formid_ascii[64]; // Only 8 should be necessary, but in case file format is corrupt ...
//	     rgb_ascii[16][64];

	int rgb[3];

	FILE *fp_lt;

	/*************************************************************
	 ** First things first, parse all files in the IMPORT_TEX_DIR 
	 ** directory and load them in to memory.
	 ************************************************************/

	lod_ltex.count = 0;

	if ((fp_lt = fopen(filename, "rb")) != 0) {
		while (fgets(s, 1024, fp_lt) != NULL) {
			p = 0;
			for (i = 0; s[i+p] != ',' && s[i+p] != '\0'; formid_ascii[i] = s[i+p], i++);
			formid_ascii[i] = '\0';

			p += i+1;
			for (i = 0; s[i+p] != ',' && s[i+p] != '\0' && s[i+p] != '\n' && s[i+p] != '\r' && i < 128; fname[i] = s[i+p], i++);
			fname[i] = '\0';

			for (t = 0; t < ftex.count; t++) {
				if (strcmp(fname, ftex.filename[t]) == 0) {
					break;
				}
			}

			if (t == ftex.count) {
				fprintf(stderr, "WARNING: I cannot find the BMP equivalent texture of %s in my textures directory (%s). I will have to apply the default texture to the land instead.\n", fname, game_textures_filepath);
				 t = 0;
			}

			StringToReverseFormID((char *)&lod_ltex.formid[lod_ltex.count], formid_ascii);

			/********************************************************************
			 * Reusing the p variable. A bit naughty, but it's free from here on.
			 ********************************************************************/

			memset(lod_ltex.rgb[lod_ltex.count], 0, 16*3);

			for (p = 0; p < 16; p++) {
				i = (int) p/4;
				j = p - (4*i);
				memcpy(&lod_ltex.rgb[lod_ltex.count][i][j], &ftex.rgb[t][p][0], 3);

			}

			if (opt_q < 4) {
				for (i = 0; i < 2; i++) {
					for (j = 0; j < 2; j++) {
						for (k = 0; k < 3; k++) {
							rgb[k] = (unsigned char) lod_ltex.rgb[lod_ltex.count][2*i][2*j][k];
							rgb[k] += (unsigned char) lod_ltex.rgb[lod_ltex.count][2*i][2*j+1][k];
							rgb[k] += (unsigned char) lod_ltex.rgb[lod_ltex.count][2*i+1][2*j][k];
							rgb[k] += (unsigned char) lod_ltex.rgb[lod_ltex.count][2*i+1][2*j+1][k];
							lod_ltex.rgb[lod_ltex.count][i][j][k] = (unsigned char) ((float) rgb[k] / 4.0f);
						}
					}
				}
			}


			if (opt_q == 1) {
				for (i = 0; i < 2; i++) {
					for (j = 0; j < 2; j++) {
						for (k = 0; k < 3; k++) {
							rgb[k] = (unsigned char) lod_ltex.rgb[lod_ltex.count][2*i][2*j][k];
							rgb[k] += (unsigned char) lod_ltex.rgb[lod_ltex.count][2*i][2*j+1][k];
							rgb[k] += (unsigned char) lod_ltex.rgb[lod_ltex.count][2*i+1][2*j][k];
							rgb[k] += (unsigned char) lod_ltex.rgb[lod_ltex.count][2*i+1][2*j+1][k];
							lod_ltex.rgb[lod_ltex.count][i][j][k] = (unsigned char) ((float) rgb[k] / 4.0f);
						}
					}
				}
			}
			lod_ltex.count++;
		}
	} else {
		fprintf(stderr, "Unable to open the LOD texture lookup file called %s. I must have this file or I don't know which RGB to use with each texture. %s\n",
			filename, strerror(errno));
		exit(1);
		return -1;
	}

	return 0;
}

int StringToFormID(char *formid, char *s)
{
        int j;
        char htmp[3];
        char c;

        for (j = 0; j < 4; j++) {
                htmp[0] = s[(2*j)];
                htmp[1] = s[(2*j)+1];
                htmp[2] = '\0';

                if (islower(htmp[0])) htmp[0] = toupper(htmp[0]);
                if (islower(htmp[1])) htmp[1] = toupper(htmp[1]);

                sscanf_s(htmp, "%X", &c);
                formid[j] = c;
	}

	return 0;
}

int StringToReverseFormID(char *formid, char *s)
{
        int j;
        char htmp[3];
        //char c;
		int c;

		//printf("%s %s\n",s,formid);

        for (j = 0; j < 4; j++) {
                htmp[0] = s[(2*j)];
                htmp[1] = s[(2*j)+1];
                htmp[2] = '\0';

                if (islower(htmp[0])) htmp[0] = toupper(htmp[0]);
                if (islower(htmp[1])) htmp[1] = toupper(htmp[1]);

                sscanf_s(htmp, "%X", &c);
                formid[3-j] = c;
        }

        return 0;
}

int StringToHex(char *shex, char *s, int size)
{
        int j;
        char htmp[3];
        char c;

        for (j = 0; j < 3; j++) {
                htmp[0] = s[(2*j)];
                htmp[1] = s[(2*j)+1];
                htmp[2] = '\0';

                if (islower(htmp[0])) htmp[0] = toupper(htmp[0]);
                if (islower(htmp[1])) htmp[1] = toupper(htmp[1]);

                sscanf(htmp, "%X", &c);
                shex[2-j] = c;
	}
	return 0;
}

int HumptyVWD()
{
	int i, j;

	char found;

	char filename_in[256],
	     filename_out[256];

	struct { 
		int formid[1024];
		int count[1024];
		int total;
	} vwd;

	struct {
		int formid;
		char pos[12];
		char rot[12];
		float scale[4];
	} vwdat;

	FILE *fp_in,
	     *fp_out;

	MKDIR(VWD_DIR);

	for (i = 0; i < cleanup_list_count; i++) {
		sprintf_s(filename_in, 256, "%s/%s.%d.%d.tmp", TMP_VWD_DIR, worldspace_lc, cleanup_list_x[i], cleanup_list_y[i]);
		if ((fp_in = fopen(filename_in, "rb")) == 0) {
			continue;
		}

		sprintf_s(filename_out, 256, "%s/%s_%d_%d.lod", VWD_DIR, worldspace_lc, cleanup_list_x[i], cleanup_list_y[i]);
		if ((fp_out = fopen(filename_out, "wb")) == 0) {
			fprintf(stderr, "Unable to open temporary VWD file for reading (%s): %s\n",
				filename_out, strerror(errno));
			CleanUp();
			exit(1);
		}
//		printf("Write DistantLOD file %d of %d: %s\n", i+1, cleanup_list_count, filename_out);

		for (j = 0; j < 1024; j++) {
			vwd.formid[j] = 0;
			vwd.count[j] = 0;
		}
		vwd.total = 0;

		/**
		 ** Get a summary count on how many FormIDs are duplicated in this cell.
		 **/

		while (fread(&vwdat, 1, 32, fp_in) == 32) {
			found = 0;
			for (j = 0; !found && j < vwd.total; j++) {
				if (vwd.formid[j] == vwdat.formid) {
					found = 1;
					break;
				}
			}

			if (!found) {
				vwd.formid[vwd.total] = vwdat.formid;
				vwd.count[vwd.total] = 1;
				vwd.total++;
			} else {
				vwd.count[j]++;
			}
		}

		/*
		 * Write out total FormID count.
		 */

		fwrite(&vwd.total, 4, 1, fp_out);

		/**
		 ** For each FormID.
		 **/

		for (j = 0; j < vwd.total; j++) {
			fwrite(&vwd.formid[j], 4, 1, fp_out);
			fwrite(&vwd.count[j], 4, 1, fp_out);

			fseek(fp_in, 0, SEEK_SET);
			while (fread(&vwdat, 1, 32, fp_in) == 32) {
				if (vwdat.formid == vwd.formid[j]) {
					fwrite(&vwdat.pos, 12, 1, fp_out);
				}
			}

			fseek(fp_in, 0, SEEK_SET);
			while (fread(&vwdat, 1, 32, fp_in) == 32) {
				if (vwdat.formid == vwd.formid[j]) {
					fwrite(&vwdat.rot, 12, 1, fp_out);
				}
			}
			fseek(fp_in, 0, SEEK_SET);
			while (fread(&vwdat, 1, 32, fp_in) == 32) {
				if (vwdat.formid == vwd.formid[j]) {
					fwrite(&vwdat.scale, 4, 1, fp_out);
				}
			}
		}
		fclose(fp_out);
		fclose(fp_in);
	}
	
}

int ParseDir(char *dirname)
{
        DIR             *ds;
        struct dirent   *dir;
	struct stat	f_stat;

	char fullname[1024];

        if ((ds = opendir(dirname)) == NULL) {
		fprintf(stderr, "Unable to open texture directory %s for reading!: %s\n",
			dirname, strerror(errno));
                return 0;
        }

        /*  struct dirent *readdir(DIR *dir);
         *
         * Read in the files from argv[1] and print 
         */

        while ((dir = readdir(ds)) != NULL) {
		if ( !(dir->d_name[0] == '.' && dir->d_name[1] == '\0') 
		 &&  !(dir->d_name[0] == '.' && dir->d_name[1] == '.' && dir->d_name[2] == '\0')) {
			sprintf_s(fullname, 1024, "%s/%s", dirname, dir->d_name);
			if (stat(fullname, &f_stat) == 0) {
				if (S_ISDIR(f_stat.st_mode)) {
					ParseDir(fullname);
				} else {
					InitLoadTexture(fullname);
				}
			} else {
				fprintf(stderr, "Unable to stat %s: %s\n",
					dir->d_name, strerror(errno));
				perror(dir->d_name);
			}
		}
        }


	closedir(ds);

        return 1;
}

int InitLoadTexture(char *filename)
{
        int i; //, j;

        int sx = 0, sy = 0;
	int Bp = 32;

        FILE *fp_in;

        char s[65536];

        short unsigned int bmp_head_size = 54;

	if (strcmp(filename + strlen(filename) - 4, ".dds") != 0 &&
	    strcmp(filename + strlen(filename) - 4, ".bmp") != 0) {
		return 1;
	}

        if ((fp_in = fopen(filename, "rb")) == NULL) {
                fprintf(stderr, "Unable to open input file (%s) for reading: %s\n",
                        filename, strerror(errno));
                exit(1);
        }

	fread(s, 2, 1, fp_in);

	if (strncmp(s, "BM", 2) != 0) {
		fprintf(stderr, "The file called %s in the TES4qLOD texture reference directory is not a BMP file. I only understand BMPs, so please convert it to this format.\n",
			filename);
		fclose(fp_in);
		return 1;
	}

        fseek(fp_in, 0, SEEK_SET);

        // Gather data from the standard sized BMP header.
        fread(s, 54, 1, fp_in);
	memcpy(&bmp_head_size, s + 10, 2);

	if (bmp_head_size != 54) {
	        fseek(fp_in, 0, SEEK_SET);
	        fread(s, bmp_head_size, 1, fp_in);
	}

	// Copy in dimensions and bit-depth to variables.
        memcpy(&sx, s+18, 4);
        memcpy(&sy, s+22, 4);
        memcpy(&Bp, s+28, 1);

	if (sx != 4 || sy != 4) {
		fprintf(stderr, "The size or bit-rate of the texture file %s is incorrect:\n"
			"I expect all BMP files to be of size 4x4, but this file is %dx%d. Please resize it ...\n", 
			filename, sx, sy);
		exit(1);
	}
        Bp = Bp / 8;

	for (i = 0; i < (sx*sy); i++) {
		fread(s, Bp, 1, fp_in);
		memcpy(&ftex.rgb[ftex.count][i][0], s, 3);
	}

	/**
	 ** Record the filename, stripping off the IMPORT_TEX_DIR from the beginning of the path.
	 **/

	memcpy(ftex.filename[ftex.count], filename + strlen(game_textures_filepath)+1, strlen(filename)-strlen(game_textures_filepath));
	memcpy(ftex.filename[ftex.count] + strlen(ftex.filename[ftex.count]) - 3, "dds", 3);

	for (i = 0; ftex.filename[ftex.count][i] != '\0'; i++) {
		if (ftex.filename[ftex.count][i] == '/') {
			ftex.filename[ftex.count][i] = '\\';
		}
	}

	ftex.count++;

	fclose(fp_in);

	return 0;
}

#endif
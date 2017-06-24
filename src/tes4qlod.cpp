#include "../include/tes4qlod.h"
#include "../../lltool/include/llutils.h"
#include "../../lltool/include/llmapworker.h"
#include "../../lltool/include/llmaplist.h"


#define MKDIR(a)	_mkdir(a)

#define TMP_TEX_DIR			"tes4qlod_partials"
#define TMP_NORMAL_DIR		"tes4qlod_normals"
#define TMP_VWD_DIR			"tes4qlod_tmp_vwd"
#define VWD_DIR				"DistantLOD"

#define FULL_LOD_MAP		"%sMap.bmp"
#define FULL_LOD_NORMAL_MAP	"%sMap_fn.bmp"

#define LOD_OUTPUT_DIR_OBLIVION		"Textures/LandscapeLOD/Generated"
#define LOD_OUTPUT_DIR_OBLIVION_DOS	"Textures\\LandscapeLOD\\Generated"
#define LOD_OUTPUT_DIR_FALLOUT3		"Textures/Landscape/lod/%s"
#define LOD_OUTPUT_DIR_FALLOUT3_DOS	"Textures\\Landscape\\lod\\%s"
#define LOD_OUTPUT_DIR_SKYRIM		"Textures/Terrain/%s"
#define LOD_OUTPUT_DIR_SKYRIM_DOS	"Textures\\Terrain\\%s"

#define LOD_LTEX_DATA_FILE			"tes4qlod_%s_ltex.dat"
#define IMPORT_TEX_DIR	            "tes4qlod_tex/%s"

#define TES4_OB_RECORD_SIZE  20
#define TES4_FA_SK_RECORD_SIZE 24

enum { EXTERIOR, INTERIOR, TRUE1, FALSE1, TEXTURES, NORMALS };


//global geometry:
int TES4qLOD::min_x =  32768;
int TES4qLOD::max_x = -32768;
int TES4qLOD::min_y =  32768;
int TES4qLOD::max_y = -32768;

//Gamemode:
char *TES4qLOD::opt_tes_mode  = NULL;
char *TES4qLOD::TES_SKYRIM    = "Skyrim";
char *TES4qLOD::TES_MORROWIND = "Morrowind";
char *TES4qLOD::TES_FALLOUT3  = "Fallout3";
char *TES4qLOD::TES_FALLOUTNV = "FalloutNV";
char *TES4qLOD::TES_OBLIVION  = "Oblivion";
char *TES4qLOD::TES_UNKNOWN   = "Unknown";

//constructor
TES4qLOD::TES4qLOD() : llWorker() {
	SetCommandName("tes4qlod");

	x_cell = y_cell = 0; //position of lower left corner 
}

int TES4qLOD::RegisterOptions(void) {
	if (!llWorker::RegisterOptions()) return 0;

	RegisterFlag ("-a", &opt_blending);
	RegisterFlag ("-d", &opt_debug);
	RegisterValue("-i", &opt_load_index);
	RegisterFlag ("-m", &opt_read_heightmap);
	RegisterValue("-q", &opt_q);
	RegisterFlag ("-x", &opt_read_dimensions);
	RegisterFlag ("-z", &opt_center);
	RegisterFlag ("-AddKeepout", &opt_keepout);

	RegisterValue("-dimX",   &opt_size_x);
	RegisterValue("-dimY",   &opt_size_y);
	RegisterValue("-x1",     &opt_x1);
	RegisterValue("-y1",     &opt_y1);
	RegisterFlag ("-silent", &silent);

	RegisterValue("-map",      &mapname);
	RegisterValue("-watermap", &watername);
	RegisterValue("-colormap", &colorname);

	return 1;
}

int TES4qLOD::Prepare(void) {
	if (!llWorker::Prepare()) return 0;

	map       = NULL;
	mapname   = NULL;
	watermap  = NULL;
	watername = NULL;
	colormap  = NULL;
	colorname = NULL;

	opt_ltex = -1;
    opt_bmp = 0;
    opt_load_index = 0;
    opt_q = 1;
    opt_x_offset = 0;
    opt_y_offset = 0;
    opt_debug = 0;
    opt_no_vclr = 0;
    opt_lod_tex = 0;
    opt_normals = 0;
	opt_blending = 0; //CHANGE_IF
	opt_read_heightmap = 0; //CHANGE_IF
	opt_read_dimensions = 0;//CHANGE_IF
	opt_size_x = 0;
	opt_size_y = 0;
	opt_center = 0;
	opt_keepout = 0;

	verbosity = 0;
	silent    = 0;

	total_refr = 0;
	total_vwd  = 0;
	total_cells = total_land = total_worlds = 0;
	in_vwd = 0;

	worldspace_found = 0;

	tes_rec_offset = TES4_OB_RECORD_SIZE; 

	return 1;
}

int TES4qLOD::Exec(void) {
	llWorker::Exec();

	cleanup_list_count = 0;

	if (!Used("-map"))
		mapname = (char *)"_heightmap";
	if (!Used("-watermap"))
		watername = (char *)"_watermap";


	//get the corresponding map from the global map container
	map      = _llMapList()->GetMap(mapname);
	watermap = _llMapList()->GetMap(watername);
	if (Used("-colormap")) 
		colormap = _llMapList()->GetMap(colorname);

	opt_install_dir = _llUtils()->GetValue("_install_dir");
	if (opt_install_dir && !strlen(opt_install_dir)) opt_install_dir = NULL;
	worldspace      = _llUtils()->GetValue("_worldspace");
	DDS_CONVERTOR   = _llUtils()->GetValue("_dds_tool");

	//--> original code starting here

	if (opt_q < 1)  opt_q = 1;
	if (opt_q == 3) opt_q = 2;
	if (opt_q > 4)  opt_q = 4;

	if (silent) verbosity = 0;

	if (opt_lod_tex == 0 && !opt_vwd && !opt_read_heightmap && !opt_read_dimensions) { opt_lod_tex = 1; }

	char tmp_dirname[1024];

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

	/*
	 * Initialize variables:
	 */

//	printf("X offset will be %d. Y will be %d\n", opt_x_offset, opt_y_offset);

	cell.name[0] = '\0';
	cell.current_x = 0;
	cell.current_y = 0;

	//DecodeFilenames(argv[argc-1]);
	//printf("esp-list decoded, I will run over %i files\n",input_files.count); //CHANGE_IF

	opt_tes_mode = TES_UNKNOWN;
	if (_llUtils()->GetValue("_gamemode")) {
		if (_stricmp(_llUtils()->GetValue("_gamemode"), TES_OBLIVION) == 0)  opt_tes_mode = TES_OBLIVION;
		if (_stricmp(_llUtils()->GetValue("_gamemode"), TES_SKYRIM) == 0)    opt_tes_mode = TES_SKYRIM;
		if (_stricmp(_llUtils()->GetValue("_gamemode"), TES_FALLOUT3) == 0)  opt_tes_mode = TES_FALLOUT3;
		if (_stricmp(_llUtils()->GetValue("_gamemode"), TES_FALLOUTNV) == 0) opt_tes_mode = TES_FALLOUTNV;
	}

	/***************************************************************************************
	 * Open the first filename - the input ESM/ESP file for reading.
	 **************************************************************************************/
	if (_llUtils()->GetNumMods())
		CheckTESVersion(_llUtils()->GetMod(0));

	char s[40];	/* For storing the 16-byte header. */
	FILE *fpin; /* Input File Stream (original ESP/ESM).      */

	if ((fpin = fopen(_llUtils()->GetMod(0), "rb")) == NULL) {
		fprintf(stderr, "Unable to open %s for reading: %s\n",
			_llUtils()->GetMod(0), strerror(errno));
		return 0;
	}

	if (fread(s, 1, 4, fpin) < 4) {
		fprintf(stderr, "Unable to read the first 4 bytes from %s to determine if it's "
			"a TES3 or TES4 file: %s\n", strerror(errno));
		return(0);
	} else {
		if (strncmp(s, "TES3", 4) == 0) {
			printf("This is a TES3: Morrowind file: But I only decode TES4: Oblivion/Fallout3/FalloutNV/Skyrim files. I only create landscape LOD textures and VisibleWhenDistant files!\n");
			exit(1);
		} else if (strncmp(s, "TES4", 4) != 0) {
			printf("This is neither a TES3 or TES4 file - the first 4 bytes do not match what I'd expect!\n");
			return(0);
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

	char lod_texture_formids_file[512];

	if (opt_lod_tex && 0) { //not required
		sprintf_s(lod_texture_formids_file, 512, LOD_LTEX_DATA_FILE, opt_tes_mode);
		if (verbosity) printf("Reading %s.esm Texture FormIDs from %s ...", opt_tes_mode, lod_texture_formids_file);
		fflush(stdout);
		
		ReadLODTextures(lod_texture_formids_file);
		if (verbosity) printf(" finished.\n\n");
	}

	if (verbosity) printf("Searching for any cells in the worldspace %s:\n\n", worldspace);

	int i;

	for (i = 0; worldspace[i] != '\0'; i++) {
		worldspace_lc[i] = tolower(worldspace[i]); //BUGBUG?
	}
	worldspace_lc[i] = '\0';
			

	/*********************************************************
	 ** Start the ESP reading and exporting for each file.
	 *********************************************************/

	for (i = 0; i < _llUtils()->GetNumMods(); i++) {
		//std::cout << _llUtils()->GetMod(i) << std::endl;
		ExportTES4LandT4QLOD(_llUtils()->GetMod(i));
	}

	if (Used("-x1")) min_x = opt_x1;
	if (Used("-y1")) min_y = opt_y1;

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
	
	//keepout polygons
	if (opt_keepout && opt_read_heightmap) {
		llPolygonList *polygons  = _llMapList()->GetPolygonList(mapname);
		llPolygonList *wpolygons = _llMapList()->GetPolygonList(watername);
		if (!polygons) {
			_llLogger()->WriteNextLine(-LOG_FATAL, "%s: no polygon list in map [%s]", command_name, mapname);
		} else {
			for (int x=min_x; x<=max_x; x++) {
				for (int y=min_y; y<=max_y; y++) {
					int found=0;
					for (int c=0; c<cleanup_list_count; c++) {
						if (cleanup_list_x[c]==x && cleanup_list_y[c]==y) 
							found=1;
					}
					if (!found) {
						float x1 = x*(*_llUtils()->GetValueF("_cellsize_x"));
						float x2 = (x+1)*(*_llUtils()->GetValueF("_cellsize_x"));
						float y1 = y*(*_llUtils()->GetValueF("_cellsize_x"));
						float y2 = (y+1)*(*_llUtils()->GetValueF("_cellsize_x"));
						char *polygon_name = new char[100];
						sprintf_s(polygon_name, 100, "cell_%i_%i", x, y);
						polygons->AddPolygon(x1, y1, x1, y2, polygon_name);
						polygons->AddVertexToPolygon(x2, y2, polygon_name);
						polygons->AddVertexToPolygon(x2, y1, polygon_name);
						if (wpolygons) {
							wpolygons->AddPolygon(x1, y1, x1, y2, polygon_name);
							wpolygons->AddVertexToPolygon(x2, y2, polygon_name);
							wpolygons->AddVertexToPolygon(x2, y1, polygon_name);
						}
					}
				}
			}
		}
	}

	if (total_cells > 0) {
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

	if (verbosity) printf_s("\nWe've finished!\n\n"
		"\tTotal Worldspaces  found:                     %d\n"
		"\tTotal CELL records found:                     %d\n"
		"\tTotal LAND records found:                     %d\n",
		total_worlds, total_cells, total_land);

	if (opt_read_dimensions && (verbosity) ) {
		printf("\nMin X:      %d",min_x);
		printf("\nMax X:      %d",max_x);
		printf("\nMin Y:      %d",min_y);
		printf("\nMax Y:      %d",max_y);
	}

	return 1;
}


int TES4qLOD::CheckTESVersion(char *_input_esp_filename) {

	/***************************************************************************************************
	** CheckTESVersion(): Manually try to work out the file format (in case -G isn't specified).
	**
	** If it's a Fallout3/NV/Skyrim file, the record size is 4 bytes longer than an Oblivion file.
	** This tries to guess it by checking imediately after the first chunk whether it's alphanumemeric.
	**************************************************************************************************/

	//std::cout << opt_tes_mode << std::endl;

	char s[4];
	FILE *fpin;

	if ((fpin = fopen(_llUtils()->GetMod(0), "rb")) == 0) {
		fprintf(stderr, "Cannot open %s for reading: %s\n", 
			_input_esp_filename, strerror(errno));
		return -1;
	}
	fread(s, 4, 1, fpin);

	if (strncmp("TES4", s, 4) == 0) {
		fseek(fpin, TES4_FA_SK_RECORD_SIZE, SEEK_SET);
		fread(s, 4, 1, fpin);
		if (isalpha(s[0]) && isalpha(s[1]) && isalpha(s[2]) && isalpha(s[3])) {
			if (verbosity) printf("'%s' looks like a TES4 (Fallout3 / FalloutNV / Skyrim) file\n", _input_esp_filename);
			if (tes_rec_offset != TES4_FA_SK_RECORD_SIZE) {
				tes_rec_offset = TES4_FA_SK_RECORD_SIZE;
			}
			if (opt_tes_mode == TES_UNKNOWN) {
				if (verbosity) printf("Going to assume Skyrim LOD type. Please override with the flag \"_gamemode=GameType\" if this is wrong\n");
				opt_tes_mode = TES_SKYRIM;
			}

		} else {
			if (verbosity) printf("'%s' looks like a TES4 Oblivion file.\n", _input_esp_filename);
			if (tes_rec_offset != TES4_OB_RECORD_SIZE) {
				tes_rec_offset = TES4_OB_RECORD_SIZE; 
			}
			if (opt_tes_mode == TES_UNKNOWN) {
				printf("Going to assume Oblivion LOD type. Please override with the flag \"_gamemode=GameType\" if this is wrong\n");
				opt_tes_mode = TES_OBLIVION;				
			}
		}
	}
	fclose(fpin);

	return 0;
}


int TES4qLOD::ExportTES4LandT4QLOD(char *_input_esp_filename) {
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

	int //i,
	    size,	   /* Size of current record.               */
	    in_group = 0,  /* Are we in a GRUP record group or not? */
	    pos = 0,
	    group_size = 0;

//	char c;		/* For command-line getopts args.     */
	char s[40];	/* For storing the record header.     */
	char s1[5] = "none";
	char *r;	/* Pointer to the Record Data.        */

	FILE *fpin; /* Input File Stream (original ESP/ESM).  */

	CheckTESVersion(_input_esp_filename);

	if ((fpin = fopen(_input_esp_filename, "rb")) == NULL) {
		fprintf(stderr, "Unable to open %s for reading: %s\n",
			_input_esp_filename, strerror(errno));
		exit(1);
	}

	while (fread(s, 1, 8, fpin) > 0) {

		if (!isalpha(s[0]) && !isalpha(s[1]) && !isalpha(s[2]) && !isalpha(s[3])) {
			printf(" - WARNING: FOUND A WILD NON-ASCII RECORD HEADER in file %s, last rec was %c%c%c%c\n", _input_esp_filename, s1[0], s1[1], s1[2], s1[3]);

			return 0;
		}

		s1[0] = s[0];
		s1[1] = s[1];
		s1[2] = s[2];
		s1[3] = s[3];

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
			strncmp(s, "INCC", 4) == 0 ||
			strncmp(s, "ONAM", 4) == 0 ||
			strncmp(s, "SNAM", 4) == 0) {

			size = 0;
			memcpy(&size, (s+4), 2);
			size += 6;

		} else {
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
				size, _input_esp_filename, strerror(errno));
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
			} else if (!_llUtils()->IsEnabled("_modname")) {
				_llUtils()->SetValue("_modname", _input_esp_filename);
			}
		} else if (strncmp(s, "LAND", 4) == 0) {
			total_land++;
			Process4LANDData(r, size);
		} else if (opt_lod_tex && ((strncmp(s, "LTEX", 4) == 0) || strncmp(s, "TXST", 4) == 0))  {
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


int TES4qLOD::Add4LTEXData(char *_r, int _size) {

	/********************************************************************
	** Add4LTEXData():
	** 	Processes a TES4 LTEX record, extracts the FormID and texture 
	**	filename and if it doesn't currently exist in the list of
	**	used FormIDs, adds it.
	**	If the FormID is already used, the RGB data should be replaced
	**	in memory (the user may decide to replace an OB texture with a 
	**	nicer file).
	*********************************************************************/

	int  i, ii;
	int  p, j, k, t;
	int  nsize;
	int  pos = 0;
	int  formid = 0;
	int  txst_formid = 0;
	int  rgb[3];
//	char tmp_int[5];

	char fname[512];

	memcpy(&formid, _r + 12, 4);

	pos += tes_rec_offset;
	int ltex = 1;

	while (pos < _size) {
		nsize = 0;
		memcpy(&nsize, _r+pos+4, 2);

		if (strncmp("ICON", _r + pos, 4) == 0) {
			memcpy(fname, _r + pos + 6, nsize);
		} else if (strncmp("TNAM", _r + pos, 4) == 0) {
			memcpy(&txst_formid, _r + pos + 6, 4);
		} else if (strncmp("TX00", _r + pos, 4) == 0) {
			ltex = 0;
			if (opt_tes_mode == TES_SKYRIM) {
				memcpy(fname, _r + pos + 6, nsize - 4);
				fname[nsize - 5] = '\0';
				strcat_s(fname, 512, ".dds");
			} else if (opt_tes_mode == TES_FALLOUTNV || opt_tes_mode == TES_FALLOUT3) {
				memcpy(fname, _r + pos + 6, nsize);
			} 
			//std::cout << "TX00 " << fname << std::endl;
		} else if (strncmp("EDID", _r + pos, 4) == 0) {
			if (opt_tes_mode == TES_SKYRIM) {
				memcpy(fname, _r + pos + 7, nsize - 1);
			} else {
				memcpy(fname, _r + pos + 6, nsize);
			}
			strcat_s(fname, 512, ".dds");
		}

		pos += 6 + nsize;
	}

	char *myfname = fname;
	if (!ltex) {
		if (strlen(fname)>8 && strnicmp(fname, "landscape", 9)) return 0;
		myfname = fname + 10;
	} 

	/***************************************************************************
	 ** Disabled, but useful to generating that initial DAT file for a new game.
	{
		unsigned char hex_formid[4];
		memcpy(&hex_formid, &formid, 4);
		printf("DAT:%2.2X%2.2X%2.2X%2.2X,%s\n", hex_formid[3], hex_formid[2], hex_formid[1], hex_formid[0], fname);
	}
	***************************************************************************/

	if (myfname[0] == '\0') {
		printf("??? This is really weird. I found an LTEX/TXST record in your ESM/ESP file with no texture filename associated with it!! Ignoring ...\n");
	}

	for (i = 0; i < lod_ltex.count; i++) {
		if (lod_ltex.formid[i] == formid) {
			break;
		}
	}
	if (i == lod_ltex.count) {
		//std::cout << "seeking for " << myfname << std::endl;
		for (t = 0; t < ftex.count; t++) {
			//printf("Comparing LTEX %s to %s\n", ftex.filename[t], myfname);
			if (_stricmp(ftex.filename[t], myfname) == 0) {
				break;
			}
		}
		if (txst_formid) {
			//std::cout << "found txst link for " << myfname << " (" << formid << ")" << " with formid " << txst_formid << std::endl;
			for (ii = 0; ii < lod_ltex.count; ii++) {
				if (lod_ltex.formid[ii] == txst_formid) {
					//std::cout << "found the texture" << std::endl;
					lod_ltex.formid[lod_ltex.count] = formid;
					lod_ltex.txst_formid[lod_ltex.count] = txst_formid;
					for (p = 0; p < 16; p++) {
						for (k = 0; k < 3; k++) {
							for (j = 0; j < 3; j++) {
								lod_ltex.rgb[lod_ltex.count][p][j][0] = lod_ltex.rgb[ii][p][j][0];
								lod_ltex.rgb[lod_ltex.count][p][j][1] = lod_ltex.rgb[ii][p][j][1];
								lod_ltex.rgb[lod_ltex.count][p][j][2] = lod_ltex.rgb[ii][p][j][2];
							}
						}
					}
					lod_ltex.count++;
				}
			}
		} else if (t == ftex.count) {
			fprintf(stdout, "Unable to find the BMP version of the texture file %s in my textures directory (%s), but your mod file has an LTEX record that thinks it should exist. :-\\ \n",
			myfname, game_textures_filepath);
		} else {
			lod_ltex.formid[lod_ltex.count] = formid;
			if (verbosity) printf("Texture %s: FormID: %d\n", myfname, formid);
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

int TES4qLOD::Process4CELLData(char *_r, int _size) {
	
	/***************************************************************
	** Process4CELLData:
	** 	Processes a TES4 CELL record, extracting name, region and any
	**	REFR (objects in this cell) records (if they exist).
	***************************************************************
	** Format:
	** CELL (4) + Length (2) + intro_data (tes_rec_offset; 20 for Oblivion, 24 for FalloutNV/Skyrim)
	** EDID (name 4) + Length (2) + NameAsString (size given by Length)
	** XCLC (4) + Length(2) + X-co-ord (4) + Y-co-ord (4)
	***************************************************************/

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
	int found_cell = 0;

	if (_llUtils()->MyIsUpper(_r[tes_rec_offset])   && _llUtils()->MyIsUpper(_r[tes_rec_offset+1]) && 
		_llUtils()->MyIsUpper(_r[tes_rec_offset+2]) && _llUtils()->MyIsUpper(_r[tes_rec_offset+3])) {
		decomp = (char *)calloc(_size, 1); //!
		memcpy(decomp, _r+tes_rec_offset, _size-tes_rec_offset);
		decomp_size = _size-tes_rec_offset;

	} else {
		decomp = (char *)calloc(_size*300, 1);

		if (DecompressZLIBStream(_r+tes_rec_offset+4, _size-tes_rec_offset-4, decomp, &decomp_size) != 0) {
			fprintf(stderr, "Decoding of ZLIB compressed CELL record failed: %s\n", strerror(errno));
			return -1;
		}
	}

	/*****************************************
	 ****************************************/

	int   has_water  = 0;
	float waterlevel = 0.0;

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
			found_cell = 1;
		} else if (strncmp("EDID", decomp + pos, 4) == 0) {
			strncpy(cell.name, decomp + pos + 6, nsize);
		} else if (strncmp("XCLW", decomp + pos, 4) == 0) {
			memcpy(&waterlevel, decomp+pos+6, 4);
			has_water  = 1;
		} 
		pos += 6 + nsize;
	}
	 
	if (waterlevel > 100000.f || waterlevel < -100000.f) has_water = 0;

	//if (has_water && waterlevel > (5000) && found_cell) {
		//printf("found water %f %i %i %s\n", waterlevel, cell.current_x, cell.current_y, cell.name);
	//}

	if (has_water && opt_read_heightmap && watermap && found_cell) {
		//printf("%i %i\n", (cell.current_x * 3 - x_cell * 3 + 1), (cell.current_y * 3 - y_cell * 3 + 1));
		watermap->SetElementRaw(cell.current_x - x_cell, cell.current_y - y_cell, waterlevel);
	}

	cleanup_list_x[cleanup_list_count]   = cell.current_x;
	cleanup_list_y[cleanup_list_count++] = cell.current_y;

	free(decomp);

	return 0;
}

int TES4qLOD::Process4WRLDData(char *_r, int _size) {

	/*****************************************************************
	** Process4WRLDData(): Process a TES4 Worldspace record.
	*****************************************************************
	** WRLD (4 bytes) + Length (2 bytes) + EDID (name)
	****************************************************************/

	int  i;
	int  nsize;
	int  pos = 0,
	     xypos = 0;

	char lod2_dir[256];

	pos += tes_rec_offset;

	/***********************************************
	 * EDID (name of the Worldspace (6+Name bytes)).
	 **********************************************/
	if (strncmp("EDID", _r + pos, 4) == 0) {
		nsize = 0;
		memcpy(&nsize, _r+pos+4, 2);
		pos += 6;
		strncpy_s(cell.worldspace_name, 1024, _r + pos, nsize);
		pos += nsize;
		if (verbosity) printf("\nFound Worldspace: %s (FormID: %2.2X%2.2X%2.2X%2.2X)", 
			cell.worldspace_name,
			(unsigned char) (_r[15]),
			(unsigned char) (_r[14]),
			(unsigned char) (_r[13]),
			(unsigned char) (_r[12]));

		for (i = 0; cell.worldspace_name[i] != '\0'; i++) {
			cell.worldspace_name[i] = tolower(cell.worldspace_name[i]);
		} 

		if (strcmp(cell.worldspace_name, worldspace_lc) == 0) {
			if (verbosity) putchar('\n');
			memcpy(&cell.worldspace_formid, _r+12, 4);
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

			while (pos < _size) { 
				//std::cout << *(_r+pos) << *(_r+pos+1) << *(_r+pos+2) << *(_r+pos+3) << std::endl;
				if (strncmp("NAM4", _r + pos, 4) == 0) { //LOD water height
					memcpy(&waterheight, _r + pos + 6, 4);
					//std::cout << "Waterheight set to: " << waterheight << std::endl;
				}
				memcpy(&nsize, _r + pos + 4, 2);
				pos += 6 + nsize;
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


int TES4qLOD::Process4LANDData(char *_r, int _size) {

	/*****************************************************************
	** 5. Process4LandData(): Process a TES4 LAND record.
	*****************************************************************
	** LAND (4 bytes) + Length (4 bytes) + X (4 bytes) + Y (4 bytes).
	****************************************************************/

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

	unsigned char c,
	     vtxt[9][34][34][4],
	     vimage[136][136][3],
	     atxt[4],
	     vclr[33][33][3];
	char tmp_land_filename[64],
	     *decomp;

	short int quad, t;
	char unknown;

	float vtxt_opacity;

	float tex_opacity[9][34][34];
	int num_layer[34][34];
//	float my_rgb[4];
	float add_tex_opacity[136][136], add_rgb[136][136][3], tex_opacity_a, tex_opacity_b, tex_opacity_c;

	FILE *fp_land;

	for (i=0;i<34;i++) for (j=0;j<34;j++) num_layer[i][j]=-1;

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

	if (_llUtils()->MyIsUpper(_r[tes_rec_offset])   && _llUtils()->MyIsUpper(_r[tes_rec_offset+1]) && 
		_llUtils()->MyIsUpper(_r[tes_rec_offset+2]) && _llUtils()->MyIsUpper(_r[tes_rec_offset+3])) {
		decomp = (char *) calloc(_size, 1);
		memcpy(decomp, _r+tes_rec_offset, _size-tes_rec_offset);
		decomp_size = _size-tes_rec_offset;

	} else {
		decomp = (char *) calloc(_size*300, 1);

		if (DecompressZLIBStream(_r+tes_rec_offset+4, _size-tes_rec_offset-4, decomp, &decomp_size) != 0) {
			fprintf(stderr, "Decoding of ZLIB compressed CELL record failed: %s\n", strerror(errno));
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

	if (opt_read_heightmap) {
		pos = 3289-6;
		if (pos < decomp_size && strncmp("VHGT", decomp + pos, 4) == 0) {
			float start_height;
			memcpy(&start_height, decomp + pos + 6, 4);
			//cout << start_height << ":";
			for (y = 0; y < 33; y++) {
				start_height += decomp[pos + 6 + 4 + y*33];
				float running_height = start_height;
				for (x = 0; x < 33; x++) {
					if (x!=0)
						running_height += decomp[pos+6+x+4+y*33];
					if (map) {
						map->SetElementRaw((cell.current_x * 32 + x - x_cell *32 /* - 1 */), 
							(cell.current_y * 32 + y - y_cell *32 /* - 1 */), running_height);
					}
				}
			}
		}
	}

	if (!opt_lod_tex || !colormap) {
		free(decomp);
		return 0;
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

	memset(vtxt[0], 0, sizeof(vtxt));
	for (i = 0; i < 34; i++) {
		for (j = 0; j < 34; j++) {
			tex_opacity[0][i][j] = 1.0f;
			for (k = 0; k < 8; k++) tex_opacity[k+1][i][j] = 0.f; //CHANGE_IF
		}
	}

	int deb = 0;
	//if (cell.current_x == -6 && cell.current_y == -7) deb = 1;

	int found_color = 0;
	while (pos < decomp_size) {
		nsize = (unsigned char) decomp[pos + 4] + 256 * ((unsigned char) decomp[pos+5]);
		if (strncmp("ATXT", decomp+pos, 4) == 0) {
			found_color = 1;
			memcpy(atxt, decomp+pos+6, 4);
			quad = decomp[pos+6+4];
			unknown = decomp[pos+6+5];
			layer = decomp[pos+6+6]; //Get layer number, IF
			if (deb) printf("ATXT %x %x %x %x layer %i\n", atxt[3], atxt[2], atxt[1], atxt[0], layer);
			if (atxt[0] == 0 && atxt[1] == 0 && atxt[2] == 0 && atxt[3] == 0) {
				atxt[0] = atxt[1] = atxt[2] = atxt[3] = 255;
			}
			if (layer > 7) {
				printf("Found strange layer number %i\n",layer);
				layer = 7;
			}
		} else if (strncmp("BTXT", decomp+pos, 4) == 0) {
			layer = -1;
			found_color = 1;
			memcpy(atxt, decomp+pos+6, 4);
			quad = decomp[pos+6+4];
			unknown = decomp[pos+6+5];
			if (deb) printf("BTXT %x %x %x %x\n", atxt[3], atxt[2], atxt[1], atxt[0]);
			if (quad == 0) {
				for (i = 0; i < 17 ; i++) {
					for (j = 0; j < 17; j++) {
						//if (vtxt[0][i][j][0] == 0 && vtxt[0][i][j][1] == 0 && vtxt[0][i][j][2] == 0 && vtxt[0][i][j][3] == 0) {
							memcpy(vtxt[0][i][j], atxt, 4);
							tex_opacity[0][i][j] = 1.0;
							num_layer[i][j] = 0;
						//} 
					}
				}
			} else if (quad == 1) {
				for (i = 0; i < 17; i++) {
					for (j = 17; j < 34; j++) {
						//if (vtxt[0][i][j][0] == 0 && vtxt[0][i][j][1] == 0 && vtxt[0][i][j][2] == 0 && vtxt[0][i][j][3] == 0) {
							memcpy(vtxt[0][i][j], atxt, 4);
							tex_opacity[0][i][j] = 1.0;
							num_layer[i][j] = 0;
						//}
					}
				}
			} else if (quad == 2) {
				for (i = 17; i < 34 ; i++) {
					for (j = 0; j < 17; j++) {
						//if (vtxt[0][i][j][0] == 0 && vtxt[0][i][j][1] == 0 && vtxt[0][i][j][2] == 0 && vtxt[0][i][j][3] == 0) {
							memcpy(vtxt[0][i][j], atxt, 4);
							tex_opacity[0][i][j] = 1.0;
							num_layer[i][j] = 0;
						//}
					}
				}
			} else if (quad == 3) {
				for (i = 17; i < 34; i++) {
					for (j = 17; j < 34; j++) {
						//if (vtxt[0][i][j][0] == 0 && vtxt[0][i][j][1] == 0 && vtxt[0][i][j][2] == 0 && vtxt[0][i][j][3] == 0) {
							memcpy(vtxt[0][i][j], atxt, 4);
							tex_opacity[0][i][j] = 1.0;
							num_layer[i][j] = 0;
						//}
					}
				}
			}
		} else if (strncmp("VTXT", decomp+pos, 4) == 0) {
			if (deb) std::cout << "VTXT " ;
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
				if (deb) std::cout << layer+1 << ":" << i << ":" << j << ":" << vtxt_opacity << " ";
				if (deb) printf(" %x %x %x %x ", vtxt[layer+1][i][j][3],vtxt[layer+1][i][j][2],vtxt[layer+1][i][j][1],vtxt[layer+1][i][j][0]);
				num_layer[i][j] = layer + 1;
			}
			if (deb) std::cout << std::endl;
		}

		pos += 6 + nsize;
	}

	//if (!found_color) {
		//free(decomp);
		//return 0;	
	//}

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

					unsigned int posx = colormap->GetRawX(128.0f * float(cell.current_x * 32)) + x + j * opt_q;
					unsigned int posy = colormap->GetRawY(128.0f * float(cell.current_y * 32)) + y + i * opt_q;

					unsigned char red, blue, green, alpha;
					colormap->GetTupel(posx, posy, &blue, &green, &red, &alpha);
					vimage[y+opt_q*i][x+opt_q*j][0] = blue;
					vimage[y+opt_q*i][x+opt_q*j][1] = green;
					vimage[y+opt_q*i][x+opt_q*j][2] = red;			
				}
			}
			int first_alpha = 0;
			for (l=0; l<=num_layer[i][j]; l++) {
				texture_matched = -1;			
				if (deb) std::cout << "layer: " << l << ":" << i << ":" << j << ":";
				if (deb) printf("%x %x %x %x\n", vtxt[l][i][j][3],vtxt[l][i][j][2],vtxt[l][i][j][1],vtxt[l][i][j][0]);
				if ((vtxt[l][i][j][0] == 255 && vtxt[l][i][j][1] == 255 && vtxt[l][i][j][2] == 255 && vtxt[l][i][j][3] == 255)) {
					texture_matched = -2; //FONV NULL Reference
				} else {
					for (k = 0; k < lod_ltex.count; k++) {			
						if (memcmp(&vtxt[l][i][j], &lod_ltex.formid[k], 4) == 0) {
							texture_matched = k;
							if (deb) std::cout << "matched" << std::endl;
						} else if (vtxt[l][i][j][3] != 0 && lod_ltex.formid[k] >= 16777216 && memcmp(&vtxt[l][i][j], &lod_ltex.formid[k], 3) == 0) {
							texture_matched = k;
							if (deb) std::cout << "matched" << std::endl;
						} 
					}
				}

				if (texture_matched != -1) {
					k = texture_matched;
					for (y = 0; y < opt_q; y++) {
						for (x = 0; x < opt_q; x++) {
							if (l==0) { //Base texture					
								if (texture_matched >= 0) {
									memcpy(&vimage[y+opt_q*i][x+opt_q*j], &lod_ltex.rgb[k][y][x], 3);	
								}
							} else {
								if (!first_alpha /* && l==6 */) { 
									first_alpha = 1;
									//memcpy(&vimage[y+opt_q*i][x+opt_q*j], &lod_ltex.rgb[k][y][x], 3);	//BUGBUG
									add_tex_opacity[y+opt_q*i][x+opt_q*j] = tex_opacity[l][i][j];
									if (texture_matched >= 0) {
										for (m = 0; m < 3; m++) {									
											add_rgb[y+opt_q*i][x+opt_q*j][m] = (float) (unsigned char) lod_ltex.rgb[k][y][x][m];
										}
									} else { //Restore original background
										unsigned int posx = colormap->GetRawX(128.0f * float(cell.current_x * 32)) + x + j * opt_q;
										unsigned int posy = colormap->GetRawY(128.0f * float(cell.current_y * 32)) + y + i * opt_q;
										unsigned char red, blue, green, alpha;
										colormap->GetTupel(posx, posy, &blue, &green, &red, &alpha);
										add_rgb[y+opt_q*i][x+opt_q*j][0] = blue;
										add_rgb[y+opt_q*i][x+opt_q*j][1] = green;
										add_rgb[y+opt_q*i][x+opt_q*j][2] = red; 
									} 
								} else {
									
									if (opt_blending == 0) { //use Lightwaves "best win" method
										if (tex_opacity[l][i][j] > add_tex_opacity[y+opt_q*i][x+opt_q*j]) {
											for (m = 0; m < 3; m++) {
												add_tex_opacity[y+opt_q*i][x+opt_q*j] = tex_opacity[l][i][j];
												add_rgb[y+opt_q*i][x+opt_q*j][m] = (float) (unsigned char) lod_ltex.rgb[k][y][x][m];
											}
										}
									} else {
										//https://en.wikipedia.org/wiki/Alpha_compositing
										tex_opacity_b = (tex_opacity[l][i][j]);	
										tex_opacity_a = add_tex_opacity[y+opt_q*i][x+opt_q*j];
										tex_opacity_c = tex_opacity_a + (1.0f - tex_opacity_a)*tex_opacity_b;
										add_tex_opacity[y+opt_q*i][x+opt_q*j] = tex_opacity_c;
										if (tex_opacity_c) {
											if (texture_matched >= 0) {
												for (m = 0; m < 3; m++) {
													add_rgb[y+opt_q*i][x+opt_q*j][m] = (1.f/tex_opacity_c) * (
														tex_opacity_a*add_rgb[y+opt_q*i][x+opt_q*j][m] + (1.f - tex_opacity_a)*
														tex_opacity_b*((float) (unsigned char) lod_ltex.rgb[k][y][x][m]));
												}
											} else { //Restore original background
												unsigned int posx = colormap->GetRawX(128.0f * float(cell.current_x * 32)) + x + j * opt_q;
												unsigned int posy = colormap->GetRawY(128.0f * float(cell.current_y * 32)) + y + i * opt_q;
												unsigned char red, blue, green, alpha;
												colormap->GetTupel(posx, posy, &blue, &green, &red, &alpha);
												add_rgb[y+opt_q*i][x+opt_q*j][0] = (1.f/tex_opacity_c) * (
													tex_opacity_a*add_rgb[y+opt_q*i][x+opt_q*j][0] + (1.f - tex_opacity_a)*
													tex_opacity_b*((float) blue));
												add_rgb[y+opt_q*i][x+opt_q*j][1] = (1.f/tex_opacity_c) * (
													tex_opacity_a*add_rgb[y+opt_q*i][x+opt_q*j][1] + (1.f - tex_opacity_a)*
													tex_opacity_b*((float) green));
												add_rgb[y+opt_q*i][x+opt_q*j][2] = (1.f/tex_opacity_c) * (
													tex_opacity_a*add_rgb[y+opt_q*i][x+opt_q*j][2] + (1.f - tex_opacity_a)*
													tex_opacity_b*((float) red));
											}
										}
									}
									
								}
							}
						}
					}
				} 
				//if (!texture_matched) {
					//printf("%x %x %x %x\n", vtxt[l][i][j][3],vtxt[l][i][j][2],vtxt[l][i][j][1],vtxt[l][i][j][0]);
				//}
			}
			//if (num_layer[i][j] > -1) {
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
							if (!opt_no_vclr) vimage[y+opt_q*i][x+opt_q*j][m] = 
								(unsigned char) ((float) (unsigned char) vimage[y+opt_q*i][x+opt_q*j][m] * ((float) (unsigned char) vclr[i][j][2-m]/ 255.0f));									
						}
						unsigned int posx = colormap->GetRawX(128.0f * float(cell.current_x * 32)) + x + j * opt_q;
						unsigned int posy = colormap->GetRawY(128.0f * float(cell.current_y * 32)) + y + i * opt_q;
						//if (deb) vimage[y+opt_q*i][x+opt_q*j][2] = vimage[y+opt_q*i][x+opt_q*j][1] = vimage[y+opt_q*i][x+opt_q*j][0] = 255;
						colormap->SetRed  (posx, posy, vimage[y+opt_q*i][x+opt_q*j][2]);
						colormap->SetGreen(posx, posy, vimage[y+opt_q*i][x+opt_q*j][1]);
						colormap->SetBlue (posx, posy, vimage[y+opt_q*i][x+opt_q*j][0]);
						colormap->SetAlpha(posx, posy, 255);						
					}
				}
			//}
		}
	}

	free(decomp);

	return 0;
}


int TES4qLOD::Process4REFRData(char *_r, int _size) {
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

	if (!(_r[9] & 0x80) && !opt_vwd_everything) {
		return 0;
	}

	total_refr++;
	putchar('V');

	pos += tes_rec_offset;

	/***********************************************************
	 * NAME (Base FormID) or DATA (X,Y,Z) of this Placed Object.
	 **********************************************************/
	while (pos < _size) {
		memcpy(&nsize, _r + pos + 4, 2);
		if (strncmp("NAME", _r + pos, 4) == 0) {
			memcpy(&base_formid, _r + pos + 6, 4);
		} else if (strncmp("DATA", _r + pos, 4) == 0) {
			memcpy(xdat, _r + pos + 6, 24);
		} else if (strncmp("XSCL", _r + pos, 4) == 0) {
			memcpy(&scale, _r + pos + 6, 4);
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
		exit(1);
	}

	fwrite(&base_formid, 4, 1, fp);
	fwrite(xdat, 24, 1, fp);
	fwrite(&scale, 4, 1, fp);

	fclose(fp);

	return 0;
}


int TES4qLOD::DecompressZLIBStream(char *_input, int _input_size, char _output[], int *_output_size) {
	z_stream z;

	//	int count;
	int status;
	int avail_out = _input_size*60;

	z.zalloc = Z_NULL;
	z.zfree  = Z_NULL;
	z.opaque = Z_NULL;

	if (inflateInit(&z) != Z_OK) {
		fprintf(stderr, "inflateInit failed: %s\n", strerror(errno));
	}

	z.avail_in = _input_size;
	z.next_in = (Bytef*)_input;

	z.avail_out = avail_out;
	z.next_out = (Bytef*)_output;

	status = inflate( &z, Z_NO_FLUSH );
	if (status == Z_STREAM_ERROR) {
		return -1;
	}

	*_output_size += avail_out - z.avail_out;

	(void)inflateEnd(&z);

	return 0;
}


int TES4qLOD::CompressZLIBStream(char *_input, int _input_size, char _output[], int *_output_size, int _compress_level) {
	z_stream z;

//	int count;
	int status;
	int avail_out = _input_size;

	z.zalloc = Z_NULL;
	z.zfree  = Z_NULL;
	z.opaque = Z_NULL;

	if (deflateInit(&z, _compress_level) != Z_OK) {
		fprintf(stderr, "deflateInit failed: %s\n", strerror(errno));
		return -1;
	}

	z.avail_in  = _input_size;
	z.next_in   = (Bytef*) _input;
	z.avail_out = avail_out;
	z.next_out  = (Bytef*) _output;

	status = deflate( &z, 1);

	*_output_size += avail_out - z.avail_out;

	(void)deflateEnd(&z);

	return 0;
}


int TES4qLOD::ReadLODTextures(char *_filename)
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

	if ((fp_lt = fopen(_filename, "rb")) != 0) {
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
			_filename, strerror(errno));
		return -1;
	}

	return 0;
}


int TES4qLOD::StringToFormID(char *_formid, char *_s) {
	int j;
	char htmp[3];
	char c;

	for (j = 0; j < 4; j++) {
		htmp[0] = _s[(2*j)];
		htmp[1] = _s[(2*j)+1];
		htmp[2] = '\0';

		if (islower(htmp[0])) htmp[0] = toupper(htmp[0]);
		if (islower(htmp[1])) htmp[1] = toupper(htmp[1]);

		sscanf_s(htmp, "%X", &c);
		_formid[j] = c;
	}

	return 0;
}

int TES4qLOD::StringToReverseFormID(char *_formid, char *_s) {
	int  j;
	char htmp[3];
	//char c;
	int  c;

	//printf("%s %s\n",s,formid);

	for (j = 0; j < 4; j++) {
		htmp[0] = _s[(2*j)];
		htmp[1] = _s[(2*j)+1];
		htmp[2] = '\0';

		if (islower(htmp[0])) htmp[0] = toupper(htmp[0]);
		if (islower(htmp[1])) htmp[1] = toupper(htmp[1]);

		sscanf_s(htmp, "%X", &c);
		_formid[3-j] = c;
	}

	return 0;
}

int TES4qLOD::StringToHex(char *_shex, char *_s, int _size) {
	int  j;
	char htmp[3];
	char c;

	for (j = 0; j < 3; j++) {
		htmp[0] = _s[(2*j)];
		htmp[1] = _s[(2*j)+1];
		htmp[2] = '\0';

		if (islower(htmp[0])) htmp[0] = toupper(htmp[0]);
		if (islower(htmp[1])) htmp[1] = toupper(htmp[1]);

		sscanf(htmp, "%X", &c);
		_shex[2-j] = c;
	}
	return 0;
}

int TES4qLOD::HumptyVWD() {
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


int TES4qLOD::ParseDir(char *dirname) {
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


int TES4qLOD::InitLoadTexture(char *_filename) {
	int i; //, j;

	int sx = 0, sy = 0;
	int Bp = 32;

	FILE *fp_in;

	char s[65536];

	short unsigned int bmp_head_size = 54;

	if (strcmp(_filename + strlen(_filename) - 4, ".dds") != 0 &&
		strcmp(_filename + strlen(_filename) - 4, ".bmp") != 0) {
			return 1;
	}

	if ((fp_in = fopen(_filename, "rb")) == NULL) {
		fprintf(stderr, "Unable to open input file (%s) for reading: %s\n",
			_filename, strerror(errno));
		return 1;
	}

	fread(s, 2, 1, fp_in);

	if (strncmp(s, "BM", 2) != 0) {
		fprintf(stderr, "The file called %s in the TES4qLOD texture reference directory is not a BMP file. I only understand BMPs, so please convert it to this format.\n",
			_filename);
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
			_filename, sx, sy);
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

	memcpy(ftex.filename[ftex.count], _filename + strlen(game_textures_filepath)+1, strlen(_filename)-strlen(game_textures_filepath));
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

#ifndef _PTES4QLOD_H_
#define _PTES4QLOD_H_

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

#include "../../lltool/include/llworker.h"
#include "../../lltool/include/llutils.h"
#include "../../lltool/include/llmap.h"


class TES4qLOD : public llWorker {

protected:

	int TES4qLOD::CheckTESVersion(char *_input_esp_filename);
	int ExportTES4LandT4QLOD     (char *_input_esp_filename);

	int Add4LTEXData    (char *_r, int _size);
	int Process4CELLData(char *_r, int _size);
	int Process4WRLDData(char *_r, int _size);
	int Process4LANDData(char *_r, int _size);
	int Process4REFRData(char *_r, int _size);
	
	int DecompressZLIBStream(char *_input, int _input_size, char _output[], int *_output_size);
	int CompressZLIBStream  (char *_input, int _input_size, char _output[], int *_output_size, int _compress_level);

	int InitLoadTexture(char *_filename);
	int ParseDir(char *_dirname);
	int HumptyVWD();
	int StringToFormID(char *_formid, char *_s);
	int StringToReverseFormID(char *_formid, char *_s);
	int StringToHex(char *_shex, char *_s, int _size);
	int ReadLODTextures(char *_filename);


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
		int txst_formid[2048];
		char rgb[2048][4][4][3]; // 2048 sets of 2x2 arrays (containing 3 RGB bytes).
	} lod_ltex;

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

	char game_textures_filepath[256];

	int cleanup_list_x[1048576];
	int cleanup_list_y[1048576];
	int cleanup_list_count;

	int tes_rec_offset; 

	//needs wrapper to tes4ll main:
	const char *worldspace;
	char  worldspace_lc[256];
	const char *opt_install_dir;
	const char *DDS_CONVERTOR;
	int worldspace_found;
	//end wrapper

	//flags:
	int opt_ltex,
		opt_bmp,
		opt_no_dds,
		opt_no_colorlods,
		opt_no_move,
		opt_load_index,
		opt_q,
		opt_full_map,
		opt_x_offset,
		opt_y_offset,
		opt_debug,
		opt_no_vclr,
		opt_lod_tex,
		opt_normals,
		opt_lod2,
		opt_vwd,
		opt_vwd_everything,
		opt_blending, //CHANGE_IF
		opt_read_heightmap,//CHANGE_IF
		opt_read_dimensions,//CHANGE_IF
		opt_size_x,
		opt_size_y,
		opt_center,
		opt_keepout,
		opt_flip;

	int verbosity, in_vwd, silent;

	static char *opt_tes_mode;
	static char *TES_SKYRIM;
	static char *TES_MORROWIND;
	static char *TES_FALLOUT3;
	static char *TES_FALLOUTNV;
	static char *TES_OBLIVION;
	static char *TES_UNKNOWN;
	
	/***************************
	* Just some running totals.
	**************************/
	int total_refr, total_vwd, total_cells, total_land, total_worlds;	

	llMap *map;
	char  *mapname;
	llMap *watermap;
	char  *watername;
	llMap *colormap;
	char  *colorname;

public:
	
	//constructor
	TES4qLOD();

	virtual llWorker * Clone() {
		return new TES4qLOD(*this);
	}

	int Prepare(void);;
	int RegisterOptions(void);
	int Exec(void);

	static int min_x, max_x, min_y, max_y;
	int x_cell, y_cell;
	
};

#endif

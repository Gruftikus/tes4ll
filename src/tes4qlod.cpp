
#include "..\include\tes4qlod.h"

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

#define TES4_OB_RECORD_SIZE  20
#define TES4_FA_SK_RECORD_SIZE 24

enum { EXTERIOR, INTERIOR, TRUE1, FALSE1, TEXTURES, NORMALS };



//global geometry:
int TES4qLOD::min_x = 32768;
int TES4qLOD::max_x = -32768;
int TES4qLOD::min_y = 32768;
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


}

int TES4qLOD::RegisterOptions(void) {
	if (!llWorker::RegisterOptions()) return 0;

	return 1;
}

int TES4qLOD::Prepare(void) {
	if (!llWorker::Prepare()) return 0;

	opt_ltex = -1;
    opt_bmp = 0;
    opt_no_dds = 0;
	opt_no_colorlods = 0;
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
	opt_blending = 0; //CHANGE_IF
	opt_read_heightmap = 0; //CHANGE_IF
	opt_read_dimensions = 0;//CHANGE_IF
	opt_size_x = 0;
	opt_size_y = 0;
	opt_center = 0;
	opt_flip = 0;

	verbosity = 1;

	total_refr = 0;
	total_vwd  = 0;

	opt_install_dir = NULL; //BUGBUG
	worldspace    = _llUtils()->GetValue("_worldspace");
	DDS_CONVERTOR = _llUtils()->GetValue("_dds_tool");

	tes_rec_offset = TES4_OB_RECORD_SIZE; 

	return 1;
}

int TES4qLOD::Exec(void) {
	llWorker::Exec();

	cleanup_list_count = 0;
	
	return 1;
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
		CleanUp();
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


int TES4qLOD::CleanUp() {
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

int TES4qLOD::CleanUpDir(char *_dirname) {
	DIR             *dis;
	struct dirent   *dir;

	char filename[512];

	if ((dis = opendir(_dirname)) != NULL) {
		if (verbosity) printf("Cleaning up temporary directory (%s) ... ", _dirname);
		fflush(stdout);

		while ((dir = readdir(dis)) != NULL) {
			if (dir->d_name[0] != '.' && (dir->d_name[1] != '\0' || (dir->d_name[1] != '.' && dir->d_name[2] != '\0'))) {
				sprintf_s(filename, 512, "%s/%s", _dirname, dir->d_name);
				if(_unlink(filename) == -1 )
					fprintf(stdout,  "Could not delete partial %s, errno=%i\n" , filename,errno);
			}
		}
		closedir(dis);
		if (verbosity) printf("finished.\n");
		fflush(stdout);
	}

	_unlink(_dirname);

	return 0;
}


void TES4qLOD::WriteBMPHeader(FILE *_fp_out, int _sx, int _sy, int _bpp) {
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

	i = (_sx*_sy*4)+54;

	memcpy(bmp_head+2,  &i,   4);
	memcpy(bmp_head+18, &_sx, 4);
	memcpy(bmp_head+22, &_sy, 4);
	//memcpy(bmp_head+28, &bpp, 1);  // Correct the header for bits per pixel.
	bmp_head[28] = _bpp;

	i = (_sx*_sy*4);
	memcpy(bmp_head+34, &i, 4);

	for (i = 0; i < 54; i++) {
		fputc(bmp_head[i], _fp_out);
	}
}



int TES4qLOD::HumptyLODs(void) {
	//loops over the quads, the calls "HumptyLOD"
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
		
		printf("Generating a complete map of the terrain called %s\n", filename);
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

int TES4qLOD::HumptyLOD(char *_lod_filename, char *_tmp_bmp_dir, char *_fprefix, 
	int _lmin_x, int _lmax_x, int _lmin_y, int _lmax_y, int _dsize, int _invert, int _qual, int _mode) {
		//reads the partials and generates the final bmü
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

	if ((fp_o = fopen(_lod_filename, "wb")) == 0) { //CHANGE_IF (fixed typo)
		fprintf(stderr, "Cannot create a new exported bmp file (%s): %s\n",
			_lod_filename, strerror(errno));
		exit(1);
	}

	x_range = _lmax_x - _lmin_x + 1;
	y_range = _lmax_y - _lmin_y + 1;

	printf("Generating new BMP output file called: %s (Size %dx%d)\n", _lod_filename, x_range*_dsize*_qual, y_range*_dsize*_qual);

	WriteBMPHeader(fp_o, _dsize*_qual*x_range, _dsize*_qual*y_range, 24);

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
			if (_mode == TEXTURES) {
				sprintf_s(land_filename, 64, "%s/partial.%d.%d.bmp", _tmp_bmp_dir, x+_lmin_x, y+_lmin_y);
			} else {
				sprintf_s(land_filename, 64, "%s/normal.%d.%d.bmp", _tmp_bmp_dir, x+_lmin_x, y+_lmin_y);
			}
			
			if ((fp_land = fopen(land_filename, "rb")) == 0) {
				for (k = 0; k < _dsize; k++) {
					for (i = 0; i < _qual; i++) {
						if (!_invert) {
							fseek(fp_o, 54+(y*_qual*_qual*_dsize*_dsize*x_range*3) 
								+ (3*x*_qual*_dsize) + (3*i*_qual*x_range*_dsize) + (k*_qual*3*_qual*x_range*_dsize), SEEK_SET);
						} else {
							fseek(fp_o, 54+((y_range-y-1)*_qual*_qual*_dsize*_dsize*x_range*3) + (3*x*_qual*_dsize) + 
								(3*(_qual-i-1)*_qual*x_range*_dsize) + ((_dsize-k-1)*_qual*3*_qual*x_range*_dsize), SEEK_SET);
						}
						for (m = 0; m < _dsize; m++) {
							for (j = 0; j < _qual; j++) {
								if (_mode == NORMALS) {
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
				if (!_invert) {
					for (k = 0; k < _dsize; k++) {
						for (i = 0; i < _qual; i++) {
							fseek(fp_o, 54+(y*_qual*_qual*_dsize*_dsize*x_range*3) + (3*x*_qual*_dsize) + 
								(3*i*_qual*x_range*_dsize) + (k*_qual*3*_qual*x_range*_dsize), SEEK_SET);
							fread(tex_data, 3*_dsize*_qual, 1, fp_land);
							fwrite(tex_data, 3*_dsize*_qual, 1, fp_o);
						}
					}
				} else {
					for (k = 0; k < _dsize; k++) {
						for (i = (_qual) - 1; i >= 0; i--) {
							fseek(fp_o, 54+((y_range-y-1)*_qual*_qual*_dsize*_dsize*x_range*3) + (3*x*_qual*_dsize) + 
								(3*(_qual-i-1)*_qual*x_range*_dsize) + ((_dsize-k-1)*_qual*3*_qual*x_range*_dsize), SEEK_SET);
							fread(tex_data, 3*_dsize*_qual, 1, fp_land);
							fwrite(tex_data, 3*_dsize*_qual, 1, fp_o);
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

#ifndef _PLLCOMMANDS_H_
#define _PLLCOMMANDS_H_

//GUI
#define COM_GUIAPPLICATION		1
#define COM_GUIAPPLICATION_CMD	"GUITab"
#define COM_GUIECHO				2
#define COM_GUIECHO_CMD			"GUITextBox"
#define COM_GUICHECKBOX			3
#define COM_GUICHECKBOX_CMD		"GUICheckBox"
#define COM_GUIDROPDOWN			4
#define COM_GUIDROPDOWN_CMD		"GUIDropDown"
#define COM_GUIDROPDOWNITEM		5
#define COM_GUIDROPDOWNITEM_CMD	"GUIDropDownItem"
#define COM_GUIBUTTON			6
#define COM_GUIBUTTON_CMD		"GUIButton"
#define COM_GUIEXEC				7
#define COM_GUIEXEC_CMD			"GUIExec"
#define COM_GUIREQUESTVERSION   8
#define COM_GUIREQUESTVERSION_CMD	"GUIRequestVersion"

#define COM_GUIENABLE			9
#define COM_GUIENABLE_CMD		"GUIEnable"
#define COM_GUIDISABLE			10
#define COM_GUIDISABLE_CMD		"GUIDisable"

#define COM_GUISPLASHECHO		11
#define COM_GUISPLASHECHO_CMD	"GUIConsoleEcho"
#define COM_GUIMESSAGEBOX		12
#define COM_GUIMESSAGEBOX_CMD	"GUIMessageBox"
#define COM_GUIWINDOWSIZE		13
#define COM_GUIWINDOWSIZE_CMD	"GUIWindowSize"


//Focus
#define COM_FOCUSALL			20
#define COM_FOCUSALL_CMD		"FocusAll"
#define COM_FOCUSQUAD			21
#define COM_FOCUSQUAD_CMD		"FocusQuad"
#define COM_FOCUSREC			22
#define COM_FOCUSREC_CMD		"FocusRec"

//Heightmap
#define COM_FILTER					30
#define COM_FILTER_CMD				"Filter"
#define COM_READBMP					31
#define COM_READBMP_CMD				"ReadHeightmap"
#define COM_GENERATEHEIGHTMAP		32
#define COM_GENERATEHEIGHTMAP_CMD	"GenerateHeightmap"
#define COM_SETHEIGHT				33
#define COM_SETHEIGHT_CMD			"SetHeight"

//Vertex commands
#define COM_SETGRID					40
#define COM_SETGRID_CMD				"SetGrid"
#define COM_SETGRIDBORDER			41
#define COM_SETGRIDBORDER_CMD		"SetGridBorder"
#define COM_PANORAMA				42
#define COM_PANORAMA_CMD			"Panorama"
#define COM_SETSINGLEPOINT			43
#define COM_SETSINGLEPOINT_CMD		"SetVertex"
#define COM_READFILE				44
#define COM_READFILE_CMD			"ReadDataFile"
#define COM_CREATEPOLYGON			45
#define COM_CREATEPOLYGON_CMD		"CreatePolygon"
#define COM_ADDVERTEXTOPOLYGON		46
#define COM_ADDVERTEXTOPOLYGON_CMD	"AddVertexToPolygon"
#define COM_READPOLYGONDATAFILE		47
#define COM_READPOLYGONDATAFILE_CMD	"ReadPolygonDataFile"


//Triangle modifier
#define COM_DIVIDEGRID					50
#define COM_DIVIDEGRID_CMD				"DivideGrid"
#define COM_BREAKLINE					51
#define COM_BREAKLINE_CMD				"ContourLine"
#define COM_BREAKFLATTRIANGLES			52
#define COM_BREAKFLATTRIANGLES_CMD		"SplitFlatTriangles"
#define COM_REMOVEBROKENTRIANGLES		53
#define COM_REMOVEBROKENTRIANGLES_CMD	"FixBrokenTriangles"
#define COM_DIVIDEAT					54
#define COM_DIVIDEAT_CMD				"DivideAt"
#define COM_DIVIDEBETWEEN				55
#define COM_DIVIDEBETWEEN_CMD			"DivideBetween"
#define COM_DIVIDEATPOLGONBORDER		56
#define COM_DIVIDEATPOLGONBORDER_CMD	"DivideAtPolygonBorder"
#define COM_BREAKATGRID					57
#define COM_BREAKATGRID_CMD				"BreakAtGrid"
#define COM_STENCILPOLGON				58
#define COM_STENCILPOLGON_CMD			"StencilPolygon"


//misc
#define COM_SETOPTION				60
#define COM_SETOPTION_CMD			"SetOption"
#define COM_SETFLAG					61
#define COM_SETFLAG_CMD				"SetFlag"
#define COM_PARSEMODLIST			62
#define COM_PARSEMODLIST_CMD		"ParseModList"
#define COM_EXIT					63
#define COM_EXIT_CMD				"Exit"
#define COM_ADDGAME					64
#define COM_ADDGAME_CMD				"AddGame"
#define COM_SETGAMEPLUGINFILE		65
#define COM_SETGAMEPLUGINFILE_CMD	"SetGamePluginFile"
#define COM_SETGAMESEARCHPATTERN	66
#define COM_SETGAMESEARCHPATTERN_CMD	"SetGameSearchPattern"
#define COM_SETGAMESTDWS			67
#define COM_SETGAMESTDWS_CMD		"SetGameStdWorldspace"
#define COM_SETPATH					68
#define COM_SETPATH_CMD				"SetPath"
#define COM_GAMEMODE				69
#define COM_GAMEMODE_CMD			"GameMode"
#define COM_LOGFILE					70
#define COM_LOGFILE_CMD				"LogFile"

//Algos
#define COM_ALGCONST			80
#define COM_ALGCONST_CMD		"AlgConst"
#define COM_ALG1ST				81
#define COM_ALG1ST_CMD			"AlgFirstOrder"
#define COM_ALG2ND				82
#define COM_ALG2ND_CMD			"AlgSecondOrder"
#define COM_ALGSLOPE			83
#define COM_ALGSLOPE_CMD		"AlgLinear"
#define COM_ALGSTRIPE			84
#define COM_ALGSTRIPE_CMD		"AlgLayer"
#define COM_ALGPEAKFINDER		85
#define COM_ALGPEAKFINDER_CMD	"AlgPeakFinder"
#define COM_ALGRADIAL			86
#define COM_ALGRADIAL_CMD		"AlgRadial"


//Setpoints
#define COM_SETPOINTS				90
#define COM_SETPOINTS_CMD			"SetVertices"
#define COM_SETPOINTSPERQUAD		91
#define COM_SETPOINTSPERQUAD_CMD	"SetVerticesPerQuad"
#define COM_SETMAXPOINTS			92
#define COM_SETMAXPOINTS_CMD		"SetMaxVertices"
#define COM_SETMAXPOINTSPERQUAD		93
#define COM_SETMAXPOINTSPERQUAD_CMD	"SetMaxVerticesPerQuad"

//Filter
#define COM_INACTIVATEALLVERTICES		100
#define COM_INACTIVATEALLVERTICES_CMD	"InactivateAllVertices"
#define COM_ACTIVATEVISIBLEVERTICES		101
#define COM_ACTIVATEVISIBLEVERTICES_CMD	"ActivateVisibleVertices"
#define COM_REMOVEINACTIVETRIANGLES		102
#define COM_REMOVEINACTIVETRIANGLES_CMD	"RemoveInactiveTriangles"

//Triangulation
#define COM_TRIANGULATION		110
#define COM_TRIANGULATION_CMD	"MakeTriangulation"

//Writefiles
#define COM_WRITEQUAD			120
#define COM_WRITEQUAD_CMD		"WriteQuad"
#define COM_WRITEALLQUADS		121
#define COM_WRITEALLQUADS_CMD	"WriteAllQuads"
#define COM_WRITEALL			122
#define COM_WRITEALL_CMD		"WriteAll"

//Externals
#define COM_CALLTESANNWYN		130
#define COM_CALLTESANNWYN_CMD	"CallTesannwyn"
#define COM_CALLTES4QLOD		131
#define COM_CALLTES4QLOD_CMD	"CallTes4qlod"
	

#define COM_MAX_CMD				140


#define MAX_LINES				10000
#define MAX_FLAGS				2000
#define MAX_GAMES				10

#define CM_SYNTAX_ERROR			"Syntax error in [%s] in [%s]"
#define CM_UNKNOWN_OPTION		"Unknown option [%s] in [%s]"
#define CM_INVALID_OPTION		"Invalid or unknown option [%s] in [%s]"
#define CM_NO_OPTION_ALLOWED	"No option (here: [%s]) allowed for [%s]"

#include <iostream>
#include <stdarg.h>

#include "../include/lllogger.h"
#include "../include/llutils.h"


class llCommands {

 private:

    FILE * file;
	FILE * logfile;
	char * filename;
	llLogger *mesg;
	llUtils  *utils;
	char * lines[MAX_LINES];
	unsigned int num_lines, line_pointer;
	
	char * crunch_string, *crunch_saveptr, *crunch_current;

 public:

    //constructor
    llCommands(llLogger *_mesg, llUtils *_utils,  FILE *_file, char *_section = NULL);
	llCommands(llLogger *_mesg, llUtils *_utils, const char *_file, char *_section = NULL);
	llCommands(llLogger *_mesg, llUtils *_utils);
	int Reopen(char *_section);
	int Open(const char *_file, char *_section = NULL);
	int ReadCache(void);
	int ReadStdin(void);
	void ReadStdin(char *_section) {section = _section;};
	char *GetNextLine(int _cmd);
	int SaveFile(const char *_file);

    int GetCommand(void);
	void Init(void);

	int x1,y1,x2,y2; //coordinate system
    float x00,y00,x11,y11; //focus
    float gridx,gridy,z,offsetx, offsety;
	float xx1,xx2,yy1,yy2, zz1, zz2; //tool variables
    float add,multiply;
    float quadx,quady;
	float max, zmin;
    int npoints,nquadmax,mindistance,createpedestals,tes4qlod_q;
	char *tes4qlod_options, *polygon_name;
    int setmin, setmax,linear, findmin, findmax, splitwatertriangles, removebrokentriangles, ps, useshapes, onlyintracell, use16bit;
	int writeheightmap, writenormalmap, timeslice, lodshadows, usegameunits; 
	float flowfactor, flowfraction, overwrite, overdrawing;
	float quadsize_x, quadsize_y, cellsize_x, cellsize_y;
	char *texname, *datafile, *worldname, *install_dir, *dds_tool;
	char *optname;
	char *CurrentCommand;
	int size_x, size_y, center;
	float trans_x, trans_y, trans_z;
	int quadtreelevels;

	const char *game[MAX_GAMES];
	const char *plugin[MAX_GAMES];
	const char *pattern[MAX_GAMES];
	const char *std_ws[MAX_GAMES];

	float Lowest, Highest, ValueAtLowest, ValueAtHighest, Radius, Scanradius, Keepout;
	float OptRadius;

	char *section;
	int is_good;
	unsigned int current_dump_line;
	int gamemode, noskipinfo, myswitch, guienabled, dumpbatch, unselect, textinput, fileinput;
	int tes4qlod_silent, quick;
	int sameline;
	float minheight;
	float guiwidth,guiheight;

	char *guiname, *guitab, *guitext, *guihelp, *guiparent;
	char *myflagname, *myflagvalue;
	int vdist;
	int hidden;
	float bmpscaling;

	void Close(void) {
		if (logfile)
			fclose(logfile);
	}

};

#endif

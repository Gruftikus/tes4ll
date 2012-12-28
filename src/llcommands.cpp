

//#include "StdAfx.h"

#include "..\include\llcommands.h"
//#include <string.h>
#include <stdio.h>
#include <time.h>


#ifndef _MSC_VER
#include "def.h"
#else

#include <iostream>

#include <direct.h>
#include <stdlib.h>
#endif




void llCommands::Init() {
	createpedestals=0;
	useshapes=0;
	worldname="60";
	dds_tool="s3tc.exe";
	use16bit=0;
	quadsize_x = quadsize_y = 32; //quadsize in cells!
	cellsize_x = cellsize_y = 4096;
	lodshadows=0;
	overdrawing=1.0;	
	is_good=1;
	gamemode=0;
	num_lines=line_pointer=noskipinfo=0;
	minheight = -1000000;
	bmpscaling = 1.;
	size_x = size_y = center = 0;
	quadtreelevels = 1;

	for (int i=0;i<MAX_GAMES;i++) {
		game[i]=NULL;
		plugin[i]=NULL;
		pattern[i]=NULL;
		std_ws[i]=NULL;
	}
	logfile = NULL;
	CurrentCommand = NULL;
}

//constructor
llCommands::llCommands(FILE *_file, char *_section) {
	mesg     = _llLogger();
	utils    = _llUtils();
	section  = _section;
	file     = _file;
	filename = NULL;
	Init();
}

//constructor
llCommands::llCommands(const char *_file, char *_section) {
	mesg     = _llLogger();
	utils    = _llUtils();
	section  = _section;
	filename = new char[strlen(_file)];
	strcpy_s(filename,strlen(_file) + 1, _file);
	if (fopen_s(&file,filename,"r")) {
		return;
	}
	Init();
}

llCommands::llCommands() {
	mesg     = _llLogger();
	utils    = _llUtils();
	section  = "[None]";
	filename = NULL;
	file     = NULL;
	Init();
}

int llCommands::Open(const char *_file, char *_section) {
	filename = new char[strlen(_file)+1];
	strcpy_s(filename,strlen(_file) + 1, _file);
		
	if (file) {
		fclose(file);
		file = NULL;
	}
	section = _section;
	if (fopen_s(&file,filename,"r")) {
		mesg->WriteNextLine(LOG_ERROR,"Unable to open %s", filename);
		file = NULL;
		return 0;
	}
	return 1;
}


int llCommands::ReadCache(void) {
	unsigned int old_num_lines = num_lines;
	char line[10000];
	if (!file) return 0;
	while (fgets(line,10000,file)) {
		if (num_lines == MAX_LINES) {
			mesg->WriteNextLine(LOG_FATAL,"Batch file too big");
			return 0;
		}
		lines[num_lines] = new char[strlen(line)+1];
		strcpy_s(lines[num_lines],strlen(line)+1,line);
		while (*(lines[num_lines]) == ' ') (lines[num_lines])++;
		while (strlen(lines[num_lines])>1 && *(lines[num_lines]+strlen(lines[num_lines])-2) == ' ') {
			*(lines[num_lines]+strlen(lines[num_lines])-2) = '\n';
			*(lines[num_lines]+strlen(lines[num_lines])-1) = '\0';
		}
		//check if line is already there....
		int it=0;
		if (strlen(lines[num_lines])>1) {
			for (unsigned int o = 0; o < old_num_lines; o++) {
				if (strcmp(lines[num_lines],lines[o])==0) it=1;
			}
		}
		if (!it) 
			num_lines++;
	}
	fclose(file);
	file=NULL;
	return 1;
};

int llCommands::SaveFile(const char *_file) {
	FILE * wfile;
	if (fopen_s(&wfile,_file,"w")) {
		mesg->WriteNextLine(LOG_ERROR,"Unable to open %s", _file);
		return 0;
	}

	if (gamemode) {
		fprintf(wfile,"[_gamemode]\n");
		fprintf(wfile,"%s -name=\"%s\"\n",COM_GAMEMODE_CMD,game[gamemode]);
	}

	int save=1;
	for (unsigned int i=0;i<num_lines;i++) {
		if (strlen(lines[i])>1 && _strnicmp(lines[i],"[_",2)==0)
			save =0;
		else if (strlen(lines[i])>0 && _strnicmp(lines[i],"[",1)==0)
			save =1;
		if (save)
			fprintf(wfile,"%s",lines[i]);
	}
	//save all variables which are not hidden
	fprintf(wfile,"[_saved]\n");
	utils->WriteFlags(wfile);

	fclose(wfile);
	return 1;
};

int llCommands::ReadStdin(void) {
	char line[1000];
	while (gets_s(line,1000)) {
		if (num_lines == 10000) {
			mesg->WriteNextLine(LOG_FATAL,"Batch file too big");
			return 0;
		}
		lines[num_lines] = new char[strlen(line)+1];
		strcpy_s(lines[num_lines],strlen(line)+1,line);
		num_lines++;
	}
	return 1;
};

char *llCommands::GetNextLine(int _cmd) {
	if (_cmd < 0) {
		current_dump_line = 0;
		return NULL;
	}
	if (current_dump_line >= num_lines) return NULL;
	current_dump_line++;
	return lines[current_dump_line-1];
};

int llCommands::Reopen(char *_section) {
	section = _section;
	if (num_lines) {line_pointer=0;return 1;}
	if (!filename) return 0;

	if (file) {
		fclose(file);
		file = NULL;
	}
	
	if (fopen_s(&file,filename,"r")) {
		return 0;
	}
	return 1;
}


int llCommands::GetCommand(void) {

	//mesg->WriteNextLine(LOG_ERROR,"next commands, filename %s, section %s", filename,section);
	int com=-1;
	char line[10000];
	char linenew[100000];
	char *linex = line;
	size_t size=1000;
	
	if (!num_lines && file) {
		if (!fgets(line,10000,file)) {
			return -2;
		}
	} else if (num_lines) {
		if (line_pointer == num_lines) {
			return -2;
		}
		strcpy_s(line,10000,lines[line_pointer]);
		line_pointer++;
	} else {
		if (!gets_s(line,10000)) {
			return -2;
		}
	}
	

	char *ptr,*ptr2;
	char *saveptr1=NULL, *saveptr2=NULL;
	gridx=gridy=-1000000.;z=-11111111;
	xx1=xx2=yy1=yy2=zz1=zz2=-1000000.;
	offsetx=0;offsety=0;onlyintracell=0;
	add=-1.;multiply=-1.;
	overwrite=0;
	npoints=-1;
	timeslice=5;flowfactor=1;flowfraction=0.5;
	writenormalmap=writeheightmap=0;
	texname="";
	optname="";
	myflagvalue=myflagname=NULL;
	guienabled=0;
	CurrentCommand = NULL;
	usegameunits=0;

repeat:	
	for (unsigned int i=0;i<strlen(linex);i++) if (linex[i]=='\n' || linex[i]==';' || linex[i]=='#') linex[i]='\0';
	utils->StripSpaces(&linex);

	if (strlen(linex)==0) return 0;

	if (section) {
		//seeks for sections
		if (linex[0] == '[') {
			if (_stricmp(linex,section)==0) {
				is_good=1;
			}
			else is_good=0;
			return 0;
		}
		if (!is_good) return 0;
	}

#if 1
check_again:
	//check for flag replacement
	for (unsigned int i=0;i<strlen(linex);i++) {
		if (linex[i]=='$') {
			//if (i==0 || (i>1 && linex[i-1]!='\\')) {
			if (i==0 || linex[i+1]!='$' || (i>1 && linex[i-1]!='$')) {
				//find the end
				for (unsigned int j=i+1;j<strlen(linex);j++) {
					if (!(isalnum(linex[j]) || linex[j]=='_')) {
						if ((j-i) <= 1) {
							mesg->WriteNextLine(LOG_ERROR,"Something wrong in [%s]", linex);
						}
						//find the end
						linex[i]='\0';
						char tmp=linex[j];
						linex[j]='\0';
						sprintf_s(linenew,100000,"%s",linex);
						char *val=(char *)utils->GetValue(linex+i+1);
						if (strlen(val)>90000) val="<String too long>";
						sprintf_s(linenew,100000-strlen(linex),"%s%s",linenew,val);
						if (tmp != '$') sprintf_s(linenew,100000-strlen(linex)-strlen(val),"%s%c%s",linenew,tmp,linex+j+1);
						else sprintf_s(linenew,100000-strlen(linex)-strlen(val),"%s%s",linenew,linex+j+1);
						char *bla= new char[strlen(linenew)+1];
						strcpy_s(bla,strlen(linenew)+1,linenew);
						linex=bla;
						goto check_again;
					}
				}
				linex[i]='\0';
				sprintf_s(linenew,100000,"%s",linex);
				char *val=(char *)utils->GetValue(linex+i+1);
				if (strlen(val)>90000) val="<String too long>";
				sprintf_s(linenew,100000-strlen(linex),"%s%s",linenew,val);
				char *bla= new char[strlen(linenew)+i+1];
				strcpy_s(bla,strlen(linenew)+1,linenew);
				linex=bla;
				goto check_again;
			}
		}
	}
#endif

	//check for flags
	int negative=0;
	if (linex[0] == '@') {
		unsigned int i=0;
		for (i=0;i<strlen(linex);i++) {
				if (linex[i]==' ') {
					goto out;
				}
		}
out:
		if (i==strlen(linex)) {
			mesg->WriteNextLine(LOG_ERROR,"No command after: %s",linex);
			return 0;
		}
		linex[i]='\0';
		int found=0;
		if (linex[1]=='!') negative=1;

		found = (utils->IsEnabled(linex+1+negative) == 1 ? 1: 0);

		if (!found && negative==0) {
			while (*(linex+i+1) == ' ') i++;
			if (!noskipinfo) mesg->WriteNextLine(LOG_INFO,"Flag %s not set, skipped [%s]",linex+1,linex+i+1);
			return 0;
		}
		if (found && negative==1) {
			while (*(linex+i+1) == ' ') i++;
			if (!noskipinfo) mesg->WriteNextLine(LOG_INFO,"Flag %s set, skipped [%s]",linex+2,linex+i+1);
			return 0;
		}
		linex=linex+i+1;
		goto repeat;
	}

	// initialisieren und ersten Abschnitt erstellen
	ptr = strtok_int(linex, ' ', &saveptr1);
	char * cmd = ptr;

	if (!ptr) return 0;
	if (strlen(ptr)==0) return 0;

	if (_stricmp(ptr, COM_FOCUSALL_CMD)==0) {
		com = COM_FOCUSALL;
		x00 = (float)x1;
		x11 = (float)x2;
		y00 = (float)y1;
		y11 = (float)y2;
	}
	if (_stricmp(ptr, COM_FOCUSQUAD_CMD)==0) {
		com = COM_FOCUSQUAD;
		CurrentCommand = COM_FOCUSQUAD_CMD;
	}
	if (_stricmp(ptr, COM_FOCUSREC_CMD)==0) {
		com = COM_FOCUSREC;
		CurrentCommand = COM_FOCUSREC_CMD;
	}
	if (_stricmp(ptr, COM_SETGRID_CMD)==0) {
		com = COM_SETGRID;
		CurrentCommand = COM_SETGRID_CMD;
	}
	if (_stricmp(ptr, COM_SETGRIDBORDER_CMD)==0) {
		com = COM_SETGRIDBORDER;
		CurrentCommand = COM_SETGRIDBORDER_CMD;
	}
	if (_stricmp(ptr, COM_SETHEIGHT_CMD)==0) {
		com = COM_SETHEIGHT;
		CurrentCommand = COM_SETHEIGHT_CMD;
	}
	if (_stricmp(ptr, COM_DIVIDEGRID_CMD)==0) {
		com = COM_DIVIDEGRID;
		CurrentCommand = COM_DIVIDEGRID_CMD;
	}
	if (_stricmp(ptr, COM_DIVIDEAT_CMD)==0) {
		com = COM_DIVIDEAT;
		CurrentCommand = COM_DIVIDEAT_CMD;
	}
	if (_stricmp(ptr, COM_DIVIDEBETWEEN_CMD)==0) {
		com = COM_DIVIDEBETWEEN;
		CurrentCommand = COM_DIVIDEBETWEEN_CMD;
	}
	if (_stricmp(ptr, COM_DIVIDEATPOLGONBORDER_CMD)==0) {
		com = COM_DIVIDEATPOLGONBORDER;
		CurrentCommand = COM_DIVIDEATPOLGONBORDER_CMD;
	}
	if (_stricmp(ptr, COM_BREAKATGRID_CMD)==0) {
		com = COM_BREAKATGRID;
		CurrentCommand = COM_BREAKATGRID_CMD;
	}
	if (_stricmp(ptr, COM_STENCILPOLGON_CMD)==0) {
		com = COM_STENCILPOLGON;
		CurrentCommand = COM_STENCILPOLGON_CMD;
	}
	if (_stricmp(ptr, COM_CREATEPOLYGON_CMD)==0) {
		com = COM_CREATEPOLYGON;
		CurrentCommand = COM_CREATEPOLYGON_CMD;
	}
	if (_stricmp(ptr, COM_ADDVERTEXTOPOLYGON_CMD)==0) {
		com = COM_ADDVERTEXTOPOLYGON;
		CurrentCommand = COM_ADDVERTEXTOPOLYGON_CMD;
	}
	if (_stricmp(ptr, COM_BREAKLINE_CMD)==0) {
		com = COM_BREAKLINE;
		CurrentCommand = COM_BREAKLINE_CMD;
	}

	if (_stricmp(ptr, COM_ALGCONST_CMD)==0) {
		com = COM_ALGCONST;
		CurrentCommand = COM_ALGCONST_CMD;
	}
	if (_stricmp(ptr, COM_ALG1ST_CMD)==0) {
		com = COM_ALG1ST;
		CurrentCommand = COM_ALG1ST_CMD;
	}
	if (_stricmp(ptr, COM_ALG2ND_CMD)==0) {
		com = COM_ALG2ND;
		CurrentCommand = COM_ALG2ND_CMD;
	}
	if (_stricmp(ptr, COM_ALGSLOPE_CMD)==0) {
		com = COM_ALGSLOPE;
		CurrentCommand = COM_ALGSLOPE_CMD;
	}
	if (_stricmp(ptr, COM_ALGSTRIPE_CMD)==0) {
		com = COM_ALGSTRIPE;
		CurrentCommand = COM_ALGSTRIPE_CMD;
	}
	if (_stricmp(ptr, COM_ALGPEAKFINDER_CMD)==0) {
		com = COM_ALGPEAKFINDER;
		CurrentCommand = COM_ALGPEAKFINDER_CMD;
	}
	if (_stricmp(ptr, COM_ALGRADIAL_CMD)==0) {
		com = COM_ALGRADIAL;
		CurrentCommand = COM_ALGRADIAL_CMD;
	}
	
	if (_stricmp(ptr, COM_SETPOINTS_CMD)==0)  {
		com = COM_SETPOINTS;
		CurrentCommand = COM_SETPOINTS_CMD;
	}
	if (_stricmp(ptr, COM_SETPOINTSPERQUAD_CMD)==0) {
		com = COM_SETPOINTSPERQUAD;
		CurrentCommand = COM_SETPOINTSPERQUAD_CMD;
	}
	if (_stricmp(ptr, COM_SETMAXPOINTS_CMD)==0) {
		com = COM_SETMAXPOINTS;
		CurrentCommand = COM_SETMAXPOINTS_CMD;
	}
	if (_stricmp(ptr, COM_SETMAXPOINTSPERQUAD_CMD)==0) {
		com = COM_SETMAXPOINTSPERQUAD;
		CurrentCommand = COM_SETMAXPOINTSPERQUAD_CMD;
	}

	if (_stricmp(ptr, COM_SETSINGLEPOINT_CMD)==0) {
		com = COM_SETSINGLEPOINT;
		CurrentCommand = COM_SETSINGLEPOINT_CMD;
	}
	if (_stricmp(ptr, COM_READFILE_CMD)==0) {
		com = COM_READFILE;
		CurrentCommand = COM_READFILE_CMD;
	}
	if (_stricmp(ptr, COM_READPOLYGONDATAFILE_CMD)==0) {
		com = COM_READPOLYGONDATAFILE;
		CurrentCommand = COM_READPOLYGONDATAFILE_CMD;
	}
	if (_stricmp(ptr, COM_WRITEQUAD_CMD)==0) {
		com = COM_WRITEQUAD;
		CurrentCommand = COM_WRITEQUAD_CMD;
	}
	if (_stricmp(ptr, COM_WRITEALL_CMD)==0) {
		com = COM_WRITEALL;
		CurrentCommand = COM_WRITEALL_CMD;
	}
	if (_stricmp(ptr, COM_WRITEALLQUADS_CMD)==0) {
		CurrentCommand = COM_WRITEALLQUADS_CMD;
		com = COM_WRITEALLQUADS;
	}
	if (_stricmp(ptr, COM_TRIANGULATION_CMD)==0) {
		com = COM_TRIANGULATION;
		CurrentCommand = COM_TRIANGULATION_CMD;
	}
	if (_stricmp(ptr, COM_SETOPTION_CMD)==0) {
		com = COM_SETOPTION;
		CurrentCommand = COM_SETOPTION_CMD;
	}
	if (_stricmp(ptr, COM_FILTER_CMD)==0) {
		com = COM_FILTER;
		CurrentCommand = COM_FILTER_CMD;
	}
	if (_stricmp(ptr, COM_BREAKFLATTRIANGLES_CMD)==0) {
		com = COM_BREAKFLATTRIANGLES;
		CurrentCommand = COM_BREAKFLATTRIANGLES_CMD;
	}
	if (_stricmp(ptr, COM_REMOVEBROKENTRIANGLES_CMD)==0) {
		com = COM_REMOVEBROKENTRIANGLES;
		CurrentCommand = COM_REMOVEBROKENTRIANGLES_CMD;
	}
	if (_stricmp(ptr, COM_ACTIVATEVISIBLEVERTICES_CMD)==0) {
		com = COM_ACTIVATEVISIBLEVERTICES;
		CurrentCommand = COM_ACTIVATEVISIBLEVERTICES_CMD;
	}
	if (_stricmp(ptr, COM_INACTIVATEALLVERTICES_CMD)==0) {
		com = COM_INACTIVATEALLVERTICES;
		CurrentCommand = COM_INACTIVATEALLVERTICES_CMD;
	}
	if (_stricmp(ptr, COM_REMOVEINACTIVETRIANGLES_CMD)==0) {
		com = COM_REMOVEINACTIVETRIANGLES;
		CurrentCommand = COM_REMOVEINACTIVETRIANGLES_CMD;
	}
	if (_stricmp(ptr, COM_PANORAMA_CMD)==0) {
		com = COM_PANORAMA;
		CurrentCommand = COM_PANORAMA_CMD;
	}
	if (_stricmp(ptr, COM_CALLTES4QLOD_CMD)==0) {
		com = COM_CALLTES4QLOD;
		CurrentCommand = COM_CALLTES4QLOD_CMD;
	}
	if (_stricmp(ptr, COM_SETFLAG_CMD)==0) {
		com = COM_SETFLAG;
		CurrentCommand = COM_SETFLAG_CMD;
	}
	if (_stricmp(ptr, COM_ADDGAME_CMD)==0) {
		com = COM_ADDGAME;
		CurrentCommand = COM_ADDGAME_CMD;
	}
	if (_stricmp(ptr, COM_SETGAMEPLUGINFILE_CMD)==0) {
		com = COM_SETGAMEPLUGINFILE;
		CurrentCommand = COM_SETGAMEPLUGINFILE_CMD;
	}
	if (_stricmp(ptr, COM_SETGAMESEARCHPATTERN_CMD)==0) {
		com = COM_SETGAMESEARCHPATTERN;
		CurrentCommand = COM_SETGAMESEARCHPATTERN_CMD;
	}
	if (_stricmp(ptr, COM_SETGAMESTDWS_CMD)==0) {
		com = COM_SETGAMESTDWS;
		CurrentCommand = COM_SETGAMESTDWS_CMD;
	}
	if (_stricmp(ptr, COM_SETPATH_CMD)==0) {
		com = COM_SETPATH;
		CurrentCommand = COM_SETPATH_CMD;
	}
	if (_stricmp(ptr, COM_LOGFILE_CMD)==0) {
		com = COM_LOGFILE;
		CurrentCommand = COM_LOGFILE_CMD;
	}
	if (_stricmp(ptr, COM_GAMEMODE_CMD)==0) {
		com = COM_GAMEMODE;
		CurrentCommand = COM_GAMEMODE_CMD;
	}

	if (_stricmp(ptr, COM_EXIT_CMD)==0) {
		CurrentCommand = COM_EXIT_CMD;
		return COM_EXIT;
	}
	
	if (_stricmp(ptr, COM_CALLTESANNWYN_CMD)==0) {
		CurrentCommand = COM_CALLTESANNWYN_CMD;
		return COM_CALLTESANNWYN;
	}
	if (_stricmp(ptr, COM_READBMP_CMD)==0) {
		com = COM_READBMP;
		CurrentCommand = COM_READBMP_CMD;
	}
	if (_stricmp(ptr, COM_GENERATEHEIGHTMAP_CMD)==0) {
		com = COM_GENERATEHEIGHTMAP;
		CurrentCommand = COM_GENERATEHEIGHTMAP_CMD;
	}
	if (_stricmp(ptr, COM_PARSEMODLIST_CMD)==0) {
		CurrentCommand = COM_PARSEMODLIST_CMD;
		return COM_PARSEMODLIST;
	}

	if (_stricmp(ptr, COM_GUIAPPLICATION_CMD)==0) {
		com = COM_GUIAPPLICATION;
		CurrentCommand = COM_GUIAPPLICATION_CMD;
	}
	if (_stricmp(ptr, COM_GUIECHO_CMD)==0) {
		com = COM_GUIECHO;
		CurrentCommand = COM_GUIECHO_CMD;
	}
	if (_stricmp(ptr, COM_GUICHECKBOX_CMD)==0) {
		com = COM_GUICHECKBOX;
		CurrentCommand = COM_GUICHECKBOX_CMD;
	}
	if (_stricmp(ptr, COM_GUIDROPDOWN_CMD)==0) {
		com = COM_GUIDROPDOWN;
		CurrentCommand = COM_GUIDROPDOWN_CMD;
	}
	if (_stricmp(ptr, COM_GUIDROPDOWNITEM_CMD)==0) {
		com = COM_GUIDROPDOWNITEM;
		CurrentCommand = COM_GUIDROPDOWNITEM_CMD;
	}
	if (_stricmp(ptr, COM_GUIENABLE_CMD)==0) {
		com = COM_GUIENABLE;
		CurrentCommand = COM_GUIENABLE_CMD;
	}
	if (_stricmp(ptr, COM_GUIDISABLE_CMD)==0) {
		com = COM_GUIDISABLE;
		CurrentCommand = COM_GUIDISABLE_CMD;
	}
	if (_stricmp(ptr, COM_GUISPLASHECHO_CMD)==0) {
		com = COM_GUISPLASHECHO;
		CurrentCommand = COM_GUISPLASHECHO_CMD;
	}
	if (_stricmp(ptr, COM_GUIBUTTON_CMD)==0) {
		com = COM_GUIBUTTON;
		CurrentCommand = COM_GUIBUTTON_CMD;
	}
	if (_stricmp(ptr, COM_GUIEXEC_CMD)==0) {
		com = COM_GUIEXEC;
		CurrentCommand = COM_GUIEXEC_CMD;
	}
	if (_stricmp(ptr, COM_GUIMESSAGEBOX_CMD)==0) {
		com = COM_GUIMESSAGEBOX;
		CurrentCommand = COM_GUIMESSAGEBOX_CMD;
	}
	if (_stricmp(ptr, COM_GUIWINDOWSIZE_CMD)==0) {
		com = COM_GUIWINDOWSIZE;
		CurrentCommand = COM_GUIWINDOWSIZE_CMD;
	}
	if (_stricmp(ptr, COM_GUIREQUESTVERSION_CMD)==0) {
		com = COM_GUIREQUESTVERSION;
		CurrentCommand = COM_GUIREQUESTVERSION_CMD;
	}

	if (com==-1) {
		mesg->WriteNextLine(LOG_ERROR,"Unknown command [%s]",ptr);
		return com;
	}

	ptr = strtok_int(NULL,' ', &saveptr1);

	quadx=-1111,quady=-1111;
	Lowest=-2000; Highest=8000; ValueAtLowest=0.2f; ValueAtHighest=1.0f; Radius=4096; OptRadius=-1;
	Scanradius=8192;Keepout=3*4096;
	setmin=setmax=linear=findmin=findmax=splitwatertriangles=removebrokentriangles=ps=0;
	mindistance=-1;
	nquadmax=-1;
	max=-1; zmin=0;
	datafile="";
	tes4qlod_q=1;
	guiname="<Unknown>";
	guitab="<Unknown>";
	guitext="<Undefined>";
	guihelp="No help available";
	myswitch=0;
	dumpbatch=0;
	unselect=0;
	hidden=0;
	npoints=-1;
	vdist=0;
	textinput=fileinput=0;
	sameline=0;
	guiwidth=1;
	guiheight=18;
	tes4qlod_silent=0;
	quick=0;
	trans_x = trans_y = trans_z = 0.f;
	polygon_name = NULL;

	while(ptr != NULL) {
		//saveptr2 = NULL;

		if (com == COM_ADDGAME) {			
			CurrentCommand = COM_ADDGAME_CMD;
			ptr2 = strtok_int(ptr, '=',&saveptr2);
			if (ptr2!=NULL && strlen(ptr2)>0) {
				if (_stricmp(ptr2,"-n")==0) {
					ptr2 = strtok_int(NULL, '=',&saveptr2);
					if (ptr2)
						sscanf_s(ptr2,"%i",&npoints);
					else {
						mesg->WriteNextLine(LOG_ERROR,CM_SYNTAX_ERROR,ptr,CurrentCommand);return com;
					}
				} else if (_stricmp(ptr2,"-name")==0) {
					ptr2 = strtok_int(NULL, '=',&saveptr2);
					if (ptr2) {
						myflagname=new char[strlen(ptr2)+1];strcpy_s(myflagname,strlen(ptr2)+1,ptr2);utils->StripQuot(&myflagname);
					} else {
						mesg->WriteNextLine(LOG_ERROR,CM_SYNTAX_ERROR,ptr,CurrentCommand);return com;
					}
				} else {
					mesg->WriteNextLine(LOG_ERROR,CM_UNKNOWN_OPTION,ptr,CurrentCommand);return com;
				}
			} else {
				mesg->WriteNextLine(LOG_ERROR,CM_INVALID_OPTION,ptr,CurrentCommand);return com;
			}
		}

		if (com == COM_SETGAMEPLUGINFILE) {			
			CurrentCommand = COM_SETGAMEPLUGINFILE_CMD;
			ptr2 = strtok_int(ptr, '=',&saveptr2);
			if (ptr2!=NULL && strlen(ptr2)>0) {
				if (_stricmp(ptr2,"-n")==0) {
					ptr2 = strtok_int(NULL, '=',&saveptr2);
					if (ptr2)
						sscanf_s(ptr2,"%i",&npoints);
					else {
						mesg->WriteNextLine(LOG_ERROR,CM_SYNTAX_ERROR,ptr,CurrentCommand);return com;
					}
				} else if (_stricmp(ptr2,"-value")==0) {
					ptr2 = strtok_int(NULL, '=',&saveptr2);
					if (ptr2) {
						myflagname=new char[strlen(ptr2)+1];strcpy_s(myflagname,strlen(ptr2)+1,ptr2);utils->StripQuot(&myflagname);
					} else {
						mesg->WriteNextLine(LOG_ERROR,CM_SYNTAX_ERROR,ptr,CurrentCommand);return com;
					}
				} else {
					mesg->WriteNextLine(LOG_ERROR,CM_UNKNOWN_OPTION,ptr,CurrentCommand);return com;
				}
			} 			
		}

		if (com == COM_SETGAMESEARCHPATTERN) {		
			CurrentCommand = COM_SETGAMESEARCHPATTERN_CMD;
			ptr2 = strtok_int(ptr, '=',&saveptr2);
			if (ptr2!=NULL && strlen(ptr2)>0) {
				if (_stricmp(ptr2,"-n")==0) {
					ptr2 = strtok_int(NULL, '=',&saveptr2);
					if (ptr2)
						sscanf_s(ptr2,"%i",&npoints);
					else {
						mesg->WriteNextLine(LOG_ERROR,CM_SYNTAX_ERROR,ptr,CurrentCommand);return com;
					}
				} else if (_stricmp(ptr2,"-value")==0) {
					ptr2 = strtok_int(NULL, '=',&saveptr2);
					if (ptr2) {
						myflagname=new char[strlen(ptr2)+1];strcpy_s(myflagname,strlen(ptr2)+1,ptr2);utils->StripQuot(&myflagname);
					} else {
						mesg->WriteNextLine(LOG_ERROR,CM_SYNTAX_ERROR,ptr,CurrentCommand);return com;
					}
				} else {
					mesg->WriteNextLine(LOG_ERROR,CM_UNKNOWN_OPTION,ptr,CurrentCommand);return com;
				}
			} else {
				mesg->WriteNextLine(LOG_ERROR,CM_INVALID_OPTION,ptr,CurrentCommand);return com;
			}
		}

		if (com == COM_SETGAMESTDWS) {			
			CurrentCommand = COM_SETGAMESTDWS_CMD;
			ptr2 = strtok_int(ptr, '=',&saveptr2);
			if (ptr2!=NULL && strlen(ptr2)>0) {
				if (_stricmp(ptr2,"-n")==0) {
					ptr2 = strtok_int(NULL, '=',&saveptr2);
					if (ptr2)
						sscanf_s(ptr2,"%i",&npoints);
					else {
						mesg->WriteNextLine(LOG_ERROR,CM_SYNTAX_ERROR,ptr,CurrentCommand);return com;
					}
				} else if (_stricmp(ptr2,"-value")==0) {
					ptr2 = strtok_int(NULL, '=',&saveptr2);
					if (ptr2) {
						myflagname=new char[strlen(ptr2)+1];strcpy_s(myflagname,strlen(ptr2)+1,ptr2);utils->StripQuot(&myflagname);
					} else {
						mesg->WriteNextLine(LOG_ERROR,CM_SYNTAX_ERROR,ptr,CurrentCommand);return com;
					}
				} else {
					mesg->WriteNextLine(LOG_ERROR,CM_UNKNOWN_OPTION,ptr,CurrentCommand);return com;
				}
			} 			
		}

		if (com == COM_SETPATH) {			
			CurrentCommand = COM_SETPATH_CMD;
			ptr2 = strtok_int(ptr, '=',&saveptr2);
			if (ptr2!=NULL && strlen(ptr2)>0) {
				if (_stricmp(ptr2,"-value")==0) {
					ptr2 = strtok_int(NULL, '=',&saveptr2);
					if (ptr2) {
						myflagname=new char[strlen(ptr2)+1];strcpy_s(myflagname,strlen(ptr2)+1,ptr2);utils->StripQuot(&myflagname);
					} else {
						mesg->WriteNextLine(LOG_ERROR,"Syntax error in [%s] after [%s]",ptr,CurrentCommand);;return com;
					}
				} else {
					mesg->WriteNextLine(LOG_ERROR,CM_UNKNOWN_OPTION,ptr,CurrentCommand);return com;
				}
			} else {
				mesg->WriteNextLine(LOG_ERROR,CM_INVALID_OPTION,ptr,CurrentCommand);return com;
			}
		}

		if (com == COM_LOGFILE) {			
			CurrentCommand = COM_LOGFILE_CMD;
			ptr2 = strtok_int(ptr, '=',&saveptr2);
			if (ptr2!=NULL && strlen(ptr2)>0) {
				if (_stricmp(ptr2,"-value")==0) {
					ptr2 = strtok_int(NULL, '=',&saveptr2);
					if (ptr2) {
						myflagname=new char[strlen(ptr2)+1];strcpy_s(myflagname,strlen(ptr2)+1,ptr2);utils->StripQuot(&myflagname);
					} else {
						mesg->WriteNextLine(LOG_ERROR,CM_SYNTAX_ERROR,ptr,CurrentCommand);return com;
					}
				} else {
					mesg->WriteNextLine(LOG_ERROR,CM_UNKNOWN_OPTION,ptr,CurrentCommand);return com;
				}
			} else {
				mesg->WriteNextLine(LOG_ERROR,CM_INVALID_OPTION,ptr,CurrentCommand);return com;
			}
		}

		if (com == COM_GAMEMODE) {			
			CurrentCommand = COM_GAMEMODE_CMD;
			ptr2 = strtok_int(ptr, '=',&saveptr2);
			if (ptr2!=NULL && strlen(ptr2)>0) {
				if (_stricmp(ptr2,"-name")==0) {
					ptr2 = strtok_int(NULL, '=',&saveptr2);
					if (ptr2) {
						myflagname=new char[strlen(ptr2)+1];strcpy_s(myflagname,strlen(ptr2)+1,ptr2);utils->StripQuot(&myflagname);
					} else {
						mesg->WriteNextLine(LOG_ERROR,CM_SYNTAX_ERROR,ptr,CurrentCommand);return com;
					}
				} else {
					mesg->WriteNextLine(LOG_ERROR,CM_UNKNOWN_OPTION,ptr,CurrentCommand);return com;
				}
			} else {
				mesg->WriteNextLine(LOG_ERROR,CM_INVALID_OPTION,ptr,CurrentCommand);return com;
			}
		}

		if (com == COM_SETFLAG) {
			CurrentCommand = COM_SETFLAG_CMD;
			if (_stricmp(ptr,"-hidden")==0) {
				hidden=1;
			} else if (_stricmp(ptr,"-unselect")==0) {
				unselect=1;
			} else {
				ptr2 = strtok_int(ptr, '=',&saveptr2);
				if (ptr2!=NULL && strlen(ptr2)>0) {
					if (_stricmp(ptr2,"-value")==0) {
						ptr2 = strtok_int(NULL, '=',&saveptr2);
						if (ptr2) {
							myflagvalue=new char[strlen(ptr2)+1];strcpy_s(myflagvalue,strlen(ptr2)+1,ptr2);utils->StripQuot(&myflagvalue);
						} else {
							mesg->WriteNextLine(LOG_ERROR,CM_SYNTAX_ERROR,ptr,CurrentCommand);return com;
						}
					} else if (_stricmp(ptr2,"-name")==0) {
						ptr2 = strtok_int(NULL, '=',&saveptr2);
						if (ptr2) {
							myflagname=new char[strlen(ptr2)+1];strcpy_s(myflagname,strlen(ptr2)+1,ptr2);utils->StripQuot(&myflagname);
						} else {
							mesg->WriteNextLine(LOG_ERROR,CM_SYNTAX_ERROR,ptr,CurrentCommand);return com;
						}
					} else {
						mesg->WriteNextLine(LOG_ERROR,CM_UNKNOWN_OPTION,ptr,CurrentCommand);return com;
					}
				} else {
					mesg->WriteNextLine(LOG_ERROR,CM_INVALID_OPTION,ptr,CurrentCommand);return com;
				} 
			} 
		}

		if (com == COM_GUIAPPLICATION) {
			CurrentCommand = COM_GUIAPPLICATION_CMD;
			ptr2 = strtok_int(ptr, '=',&saveptr2);
			if (ptr2!=NULL && strlen(ptr2)>0) {
				if (_stricmp(ptr2,"-name")==0) {
					ptr2 = strtok_int(NULL, '=',&saveptr2);
					if (ptr2) {
						guiname=new char[strlen(ptr2)+1];strcpy_s(guiname,strlen(ptr2)+1,ptr2);utils->StripQuot(&guiname);
					} else {
						mesg->WriteNextLine(LOG_ERROR,CM_SYNTAX_ERROR,ptr,CurrentCommand);return com;
					}
				} else if (_stricmp(ptr2,"-help")==0) {
					ptr2 = strtok_int(NULL, '=',&saveptr2);
					if (ptr2) {
						guitab=new char[strlen(ptr2)+1];strcpy_s(guitab,strlen(ptr2)+1,ptr2);utils->StripQuot(&guitab);
					} else {
						mesg->WriteNextLine(LOG_ERROR,CM_SYNTAX_ERROR,ptr,CurrentCommand);return com;
					}
				} else if (_stricmp(ptr2,"-text")==0) {
					ptr2 = strtok_int(NULL, '=',&saveptr2);
					if (ptr2) {
						guitext=new char[strlen(ptr2)+1];strcpy_s(guitext,strlen(ptr2)+1,ptr2);utils->StripQuot(&guitext);
					} else {
						mesg->WriteNextLine(LOG_ERROR,CM_SYNTAX_ERROR,ptr,CurrentCommand);return com;
					}
				} else {
					mesg->WriteNextLine(LOG_ERROR,CM_UNKNOWN_OPTION,ptr,CurrentCommand);return com;
				}
			} else {
				mesg->WriteNextLine(LOG_ERROR,CM_INVALID_OPTION,ptr,CurrentCommand);return com;
			}
		}

		if (com == COM_GUIECHO) {
			CurrentCommand = COM_GUIECHO_CMD;
			ptr2 = strtok_int(ptr, '=',&saveptr2);
			if (_stricmp(ptr,"-input")==0) {
				textinput=1;
			} else if (_stricmp(ptr,"-fileinput")==0) {
				fileinput=1;
			} else if (_stricmp(ptr,"-sameline")==0) {
				sameline=1;
			} else if (ptr2!=NULL && strlen(ptr2)>0) {
				if (_stricmp(ptr2,"-text")==0) {
					ptr2 = strtok_int(NULL, '=',&saveptr2);
					if (ptr2) {
						guitext=new char[strlen(ptr2)+1];strcpy_s(guitext,strlen(ptr2)+1,ptr2);utils->StripQuot(&guitext);
					} else {
						mesg->WriteNextLine(LOG_ERROR,CM_SYNTAX_ERROR,ptr,CurrentCommand);return com;
					}
				} else if (_stricmp(ptr2,"-name")==0) {
					ptr2 = strtok_int(NULL, '=',&saveptr2);
					if (ptr2) {
						guiname=new char[strlen(ptr2)+1];strcpy_s(guiname,strlen(ptr2)+1,ptr2);utils->StripQuot(&guiname);
					} else {
						mesg->WriteNextLine(LOG_ERROR,CM_SYNTAX_ERROR,ptr,CurrentCommand);return com;
					}
				} else if (_stricmp(ptr2,"-help")==0) {
					ptr2 = strtok_int(NULL, '=',&saveptr2);
					if (ptr2) {
						guihelp=new char[strlen(ptr2)+1];strcpy_s(guihelp,strlen(ptr2)+1,ptr2);utils->StripQuot(&guihelp);
					} else {
						mesg->WriteNextLine(LOG_ERROR,CM_SYNTAX_ERROR,ptr,CurrentCommand);return com;
					}
				} else if (_stricmp(ptr2,"-vdist")==0) {
					ptr2 = strtok_int(NULL, '=',&saveptr2);
					if (ptr2)
						sscanf_s(ptr2,"%i",&vdist);
					else {
						mesg->WriteNextLine(LOG_ERROR,CM_SYNTAX_ERROR,ptr,CurrentCommand);return com;
					}
				} else if (_stricmp(ptr2,"-width")==0) {
					ptr2 = strtok_int(NULL, '=',&saveptr2);
					if (ptr2)
						sscanf_s(ptr2,"%f",&guiwidth);
					else {
						mesg->WriteNextLine(LOG_ERROR,CM_SYNTAX_ERROR,ptr,CurrentCommand);return com;
					}
				} else if (_stricmp(ptr2,"-height")==0) {
					ptr2 = strtok_int(NULL, '=',&saveptr2);
					if (ptr2)
						sscanf_s(ptr2,"%f",&guiheight);
					else {
						mesg->WriteNextLine(LOG_ERROR,CM_SYNTAX_ERROR,ptr,CurrentCommand);return com;
					}
				} else {
					mesg->WriteNextLine(LOG_ERROR,CM_UNKNOWN_OPTION,ptr,CurrentCommand);return com;
				}
			} else {
				mesg->WriteNextLine(LOG_ERROR,CM_INVALID_OPTION,ptr,CurrentCommand);return com;
			}
		}

		if (com == COM_GUICHECKBOX) {
			CurrentCommand = COM_GUICHECKBOX_CMD;
			if (_stricmp(ptr,"-select")==0) {
				guienabled=1;
			} else if (_stricmp(ptr,"-sameline")==0) {
				sameline=1;
			} else {
				ptr2 = strtok_int(ptr, '=',&saveptr2);
				if (ptr2!=NULL && strlen(ptr2)>0) {
					if (_stricmp(ptr2,"-text")==0) {
						ptr2 = strtok_int(NULL, '=',&saveptr2);
						if (ptr2) {
							guitext=new char[strlen(ptr2)+1];strcpy_s(guitext,strlen(ptr2)+1,ptr2);utils->StripQuot(&guitext);
						} else {
							mesg->WriteNextLine(LOG_ERROR,CM_SYNTAX_ERROR,ptr,CurrentCommand);return com;
						}
					} else if (_stricmp(ptr2,"-name")==0) {
						ptr2 = strtok_int(NULL, '=',&saveptr2);
						if (ptr2) {
							guiname=new char[strlen(ptr2)+1];strcpy_s(guiname,strlen(ptr2)+1,ptr2);utils->StripQuot(&guiname);
						} else {
							mesg->WriteNextLine(LOG_ERROR,CM_SYNTAX_ERROR,ptr,CurrentCommand);return com;
						}
					} else if (_stricmp(ptr2,"-help")==0) {
						ptr2 = strtok_int(NULL, '=',&saveptr2);
						if (ptr2) {
							guihelp=new char[strlen(ptr2)+1];strcpy_s(guihelp,strlen(ptr2)+1,ptr2);utils->StripQuot(&guihelp);
						} else {
							mesg->WriteNextLine(LOG_ERROR,CM_SYNTAX_ERROR,ptr,CurrentCommand);return com;
						}
					} else if (_stricmp(ptr2,"-width")==0) {
						ptr2 = strtok_int(NULL, '=',&saveptr2);
						if (ptr2)
							sscanf_s(ptr2,"%f",&guiwidth);
						else {
							mesg->WriteNextLine(LOG_ERROR,CM_SYNTAX_ERROR,ptr,CurrentCommand);return com;
						}
					} else if (_stricmp(ptr2,"-height")==0) {
						ptr2 = strtok_int(NULL, '=',&saveptr2);
						if (ptr2)
							sscanf_s(ptr2,"%f",&guiheight);
						else {
							mesg->WriteNextLine(LOG_ERROR,CM_SYNTAX_ERROR,ptr,CurrentCommand);return com;
						}
					} else if (_stricmp(ptr2,"-vdist")==0) {
						ptr2 = strtok_int(NULL, '=',&saveptr2);
						if (ptr2)
							sscanf_s(ptr2,"%i",&vdist);
						else {
							mesg->WriteNextLine(LOG_ERROR,CM_SYNTAX_ERROR,ptr,CurrentCommand);return com;
						}
					} else {
						mesg->WriteNextLine(LOG_ERROR,CM_UNKNOWN_OPTION,ptr,CurrentCommand);return com;
					}
				} else {
					mesg->WriteNextLine(LOG_ERROR,CM_INVALID_OPTION,ptr,CurrentCommand);return com;
				}
			}
		}

		if (com == COM_GUIDROPDOWN) {
			CurrentCommand = COM_GUIDROPDOWN_CMD;
			ptr2 = strtok_int(ptr, '=',&saveptr2);
			if (_stricmp(ptr,"-sameline")==0) {
				sameline=1;
			} else if (ptr2!=NULL && strlen(ptr2)>0) {
				if (_stricmp(ptr2,"-text")==0) {
					ptr2 = strtok_int(NULL, '=',&saveptr2);
					if (ptr2) {
						guitext=new char[strlen(ptr2)+1];strcpy_s(guitext,strlen(ptr2)+1,ptr2);utils->StripQuot(&guitext);
					} else {
						mesg->WriteNextLine(LOG_ERROR,CM_SYNTAX_ERROR,ptr,CurrentCommand);return com;
					}
				} else if (_stricmp(ptr2,"-name")==0) {
					ptr2 = strtok_int(NULL, '=',&saveptr2);
					if (ptr2) {
						guiname=new char[strlen(ptr2)+1];strcpy_s(guiname,strlen(ptr2)+1,ptr2);utils->StripQuot(&guiname);
					} else {
						mesg->WriteNextLine(LOG_ERROR,CM_SYNTAX_ERROR,ptr,CurrentCommand);return com;
					}
				} else if (_stricmp(ptr2,"-help")==0) {
					ptr2 = strtok_int(NULL, '=',&saveptr2);
					if (ptr2) {
						guihelp=new char[strlen(ptr2)+1];strcpy_s(guihelp,strlen(ptr2)+1,ptr2);utils->StripQuot(&guihelp);
					} else {
						mesg->WriteNextLine(LOG_ERROR,CM_SYNTAX_ERROR,ptr,CurrentCommand);return com;
					}
				} else if (_stricmp(ptr2,"-vdist")==0) {
					ptr2 = strtok_int(NULL, '=',&saveptr2);
					if (ptr2)
						sscanf_s(ptr2,"%i",&vdist);
					else {
						mesg->WriteNextLine(LOG_ERROR,CM_SYNTAX_ERROR,ptr,CurrentCommand);return com;
					}
				} else if (_stricmp(ptr2,"-width")==0) {
					ptr2 = strtok_int(NULL, '=',&saveptr2);
					if (ptr2)
						sscanf_s(ptr2,"%f",&guiwidth);
					else {
						mesg->WriteNextLine(LOG_ERROR,CM_SYNTAX_ERROR,ptr,CurrentCommand);return com;
					}
				} else if (_stricmp(ptr2,"-height")==0) {
					ptr2 = strtok_int(NULL, '=',&saveptr2);
					if (ptr2)
						sscanf_s(ptr2,"%f",&guiheight);
					else {
						mesg->WriteNextLine(LOG_ERROR,CM_SYNTAX_ERROR,ptr,CurrentCommand);return com;
					}
				} else {
					mesg->WriteNextLine(LOG_ERROR,CM_UNKNOWN_OPTION,ptr,CurrentCommand);return com;
				}
			} else {
				mesg->WriteNextLine(LOG_ERROR,CM_INVALID_OPTION,ptr,CurrentCommand);return com;
			}
		}

		if (com == COM_GUIDROPDOWNITEM) {
			CurrentCommand = COM_GUIDROPDOWNITEM_CMD;
			if (_stricmp(ptr,"-select")==0) {
				guienabled=1;
			} else {
				ptr2 = strtok_int(ptr, '=',&saveptr2);
				if (ptr2!=NULL && strlen(ptr2)>0) {
					if (_stricmp(ptr2,"-text")==0) {
						ptr2 = strtok_int(NULL, '=',&saveptr2);
						if (ptr2) {
							guitext=new char[strlen(ptr2)+1];strcpy_s(guitext,strlen(ptr2)+1,ptr2);utils->StripQuot(&guitext);
						} else {
							mesg->WriteNextLine(LOG_ERROR,CM_SYNTAX_ERROR,ptr,CurrentCommand);return com;
						}
					} else if (_stricmp(ptr2,"-name")==0) {
						ptr2 = strtok_int(NULL, '=',&saveptr2);
						if (ptr2) {
							guiname=new char[strlen(ptr2)+1];strcpy_s(guiname,strlen(ptr2)+1,ptr2);utils->StripQuot(&guiname);
						} else {
							mesg->WriteNextLine(LOG_ERROR,CM_SYNTAX_ERROR,ptr,CurrentCommand);return com;
						}
					} else if (_stricmp(ptr2,"-parent")==0) {
						ptr2 = strtok_int(NULL, '=',&saveptr2);
						if (ptr2) {
							guiparent=new char[strlen(ptr2)+1];strcpy_s(guiparent,strlen(ptr2)+1,ptr2);utils->StripQuot(&guiparent);
						} else {
							mesg->WriteNextLine(LOG_ERROR,CM_SYNTAX_ERROR,ptr,CurrentCommand);return com;
						}
					} else {
						mesg->WriteNextLine(LOG_ERROR,CM_UNKNOWN_OPTION,ptr,CurrentCommand);return com;
					}
				} else {
					mesg->WriteNextLine(LOG_ERROR,CM_INVALID_OPTION,ptr,CurrentCommand);return com;
				}
			}
		}

		if (com == COM_GUIBUTTON) {
			CurrentCommand = COM_GUIBUTTON_CMD;
			ptr2 = strtok_int(ptr, '=',&saveptr2);
			if (ptr2!=NULL && strlen(ptr2)>0) {
				if (_stricmp(ptr2,"-text")==0) {
					ptr2 = strtok_int(NULL, '=',&saveptr2);
					if (ptr2) {
						guitext=new char[strlen(ptr2)+1];strcpy_s(guitext,strlen(ptr2)+1,ptr2);utils->StripQuot(&guitext);
					} else {
						mesg->WriteNextLine(LOG_ERROR,CM_SYNTAX_ERROR,ptr,CurrentCommand);return com;
					}
				} else if (_stricmp(ptr2,"-name")==0) {
					ptr2 = strtok_int(NULL, '=',&saveptr2);
					if (ptr2) {
						guiname=new char[strlen(ptr2)+1];strcpy_s(guiname,strlen(ptr2)+1,ptr2);utils->StripQuot(&guiname);
					} else {
						mesg->WriteNextLine(LOG_ERROR,CM_SYNTAX_ERROR,ptr,CurrentCommand);return com;
					}
				} else if (_stricmp(ptr2,"-vdist")==0) {
					ptr2 = strtok_int(NULL, '=',&saveptr2);
					if (ptr2)
						sscanf_s(ptr2,"%i",&vdist);
					else {
						mesg->WriteNextLine(LOG_ERROR,CM_SYNTAX_ERROR,ptr,CurrentCommand);return com;
					}
				} else {
					mesg->WriteNextLine(LOG_ERROR,CM_UNKNOWN_OPTION,ptr,CurrentCommand);return com;
				}
			} 
		}

		if (com == COM_GUIEXEC) {
			CurrentCommand = COM_GUIEXEC_CMD;
			if (_stricmp(ptr,"-dumpbatch")==0) {
				dumpbatch=1;
			} else {
				ptr2 = strtok_int(ptr, '=',&saveptr2);
				if (ptr2!=NULL && strlen(ptr2)>0) {
					if (_stricmp(ptr2,"-exe")==0) {
						ptr2 = strtok_int(NULL, '=',&saveptr2);
						if (ptr2) {
							guitext=new char[strlen(ptr2)+1];strcpy_s(guitext,strlen(ptr2)+1,ptr2);utils->StripQuot(&guitext);
						} else {
							mesg->WriteNextLine(LOG_ERROR,CM_SYNTAX_ERROR,ptr,CurrentCommand);return com;
						}
					} else {
						mesg->WriteNextLine(LOG_ERROR,CM_UNKNOWN_OPTION,ptr,CurrentCommand);return com;
					}
				} else {
					mesg->WriteNextLine(LOG_ERROR,CM_INVALID_OPTION,ptr,CurrentCommand);return com;
				}
			}
		}

		if (com == COM_GUIENABLE) {
			CurrentCommand = COM_GUIENABLE_CMD;
			if (_stricmp(ptr,"-unselect")==0) {
				unselect=1;
			} else if (_stricmp(ptr,"-select")==0) {
				guienabled=1;
			} else {
				ptr2 = strtok_int(ptr, '=',&saveptr2);
				if (ptr2!=NULL && strlen(ptr2)>0) {
					if (_stricmp(ptr2,"-name")==0) {
						ptr2 = strtok_int(NULL, '=',&saveptr2);
						if (ptr2) {
							guiname=new char[strlen(ptr2)+1];strcpy_s(guiname,strlen(ptr2)+1,ptr2);utils->StripQuot(&guiname);
						} else {
							mesg->WriteNextLine(LOG_ERROR,CM_SYNTAX_ERROR,ptr,CurrentCommand);return com;
						}
					} else {
						mesg->WriteNextLine(LOG_ERROR,CM_UNKNOWN_OPTION,ptr,CurrentCommand);return com;
					}
				} else {
					mesg->WriteNextLine(LOG_ERROR,CM_INVALID_OPTION,ptr,CurrentCommand);return com;
				}
			}
		}

		if (com == COM_GUIDISABLE) {
			CurrentCommand = COM_GUIDISABLE_CMD;
			if (_stricmp(ptr,"-unselect")==0) {
				unselect=1;
			} else if (_stricmp(ptr,"-select")==0) {
				guienabled=1;
			} else {
				ptr2 = strtok_int(ptr, '=',&saveptr2);
				if (ptr2!=NULL && strlen(ptr2)>0) {
					if (_stricmp(ptr2,"-name")==0) {
						ptr2 = strtok_int(NULL, '=',&saveptr2);
						if (ptr2) {
							guiname=new char[strlen(ptr2)+1];strcpy_s(guiname,strlen(ptr2)+1,ptr2);utils->StripQuot(&guiname);
						} else {
							mesg->WriteNextLine(LOG_ERROR,CM_SYNTAX_ERROR,ptr,CurrentCommand);return com;
						}
					} else {
						mesg->WriteNextLine(LOG_ERROR,CM_UNKNOWN_OPTION,ptr,CurrentCommand);return com;
					}
				} else {
					mesg->WriteNextLine(LOG_ERROR,CM_INVALID_OPTION,ptr,CurrentCommand);return com;
				}
			}
		}

		if (com == COM_GUISPLASHECHO) {
			CurrentCommand = COM_GUISPLASHECHO_CMD;
			if (_stricmp(ptr,"-switch")==0) {
				myswitch=1;
			} else {
				ptr2 = strtok_int(ptr, '=',&saveptr2);
				if (ptr2!=NULL && strlen(ptr2)>0) {
					if (_stricmp(ptr2,"-text")==0) {
						ptr2 = strtok_int(NULL, '=',&saveptr2);
						if (ptr2) {
							guitext=new char[strlen(ptr2)+1];strcpy_s(guitext,strlen(ptr2)+1,ptr2);utils->StripQuot(&guitext);
						} else {
							mesg->WriteNextLine(LOG_ERROR,CM_SYNTAX_ERROR,ptr,CurrentCommand);return com;
						}
					} else {
						mesg->WriteNextLine(LOG_ERROR,CM_UNKNOWN_OPTION,ptr,CurrentCommand);return com;
					}
				} else {
					mesg->WriteNextLine(LOG_ERROR,CM_INVALID_OPTION,ptr,CurrentCommand);return com;
				}
			}
		}

		if (com == COM_GUIMESSAGEBOX) {
			CurrentCommand = COM_GUIWINDOWSIZE_CMD;
			ptr2 = strtok_int(ptr, '=',&saveptr2);
			if (ptr2!=NULL && strlen(ptr2)>0) {
				if (_stricmp(ptr2,"-text")==0) {
					ptr2 = strtok_int(NULL, '=',&saveptr2);
					if (ptr2) {
						guitext=new char[strlen(ptr2)+1];strcpy_s(guitext,strlen(ptr2)+1,ptr2);utils->StripQuot(&guitext);
					} else {
						mesg->WriteNextLine(LOG_ERROR,CM_SYNTAX_ERROR,ptr,CurrentCommand);return com;
					}
				} else if (_stricmp(ptr2,"-title")==0) {
					ptr2 = strtok_int(NULL, '=',&saveptr2);
					if (ptr2) {
						guiname=new char[strlen(ptr2)+1];strcpy_s(guiname,strlen(ptr2)+1,ptr2);utils->StripQuot(&guiname);
					} else {
						mesg->WriteNextLine(LOG_ERROR,CM_SYNTAX_ERROR,ptr,CurrentCommand);return com;
					}
				} else {
					mesg->WriteNextLine(LOG_ERROR,CM_UNKNOWN_OPTION,ptr,CurrentCommand);return com;
				}
			} else {
				mesg->WriteNextLine(LOG_ERROR,CM_INVALID_OPTION,ptr,CurrentCommand);return com;
			}
		}

		if (com == COM_GUIWINDOWSIZE) {
			CurrentCommand = 
			ptr2 = strtok_int(ptr, '=',&saveptr2);
			if (ptr2!=NULL && strlen(ptr2)>0) {
				if (_stricmp(ptr2,"-x")==0) {
					ptr2 = strtok_int(NULL, '=',&saveptr2);
					if (ptr2)
						sscanf_s(ptr2,"%f",&gridx);
					else {
						mesg->WriteNextLine(LOG_ERROR,CM_SYNTAX_ERROR,ptr,CurrentCommand);return com;
					}
				}
				else if (_stricmp(ptr2,"-y")==0) {
					ptr2 = strtok_int(NULL, '=',&saveptr2);
					if (ptr2)
						sscanf_s(ptr2,"%f",&gridy);
					else {
						mesg->WriteNextLine(LOG_ERROR,CM_SYNTAX_ERROR,ptr,CurrentCommand);return com;
					}
				} else {
					mesg->WriteNextLine(LOG_ERROR,CM_UNKNOWN_OPTION,ptr,CurrentCommand);return com;
				}
			} 
			else {
				mesg->WriteNextLine(LOG_ERROR,CM_INVALID_OPTION,ptr,CurrentCommand);return com;
			}
		}


		if (com == COM_GUIREQUESTVERSION) {			
			CurrentCommand = COM_GUIREQUESTVERSION_CMD;
			ptr2 = strtok_int(ptr, '=',&saveptr2);
			if (ptr2!=NULL && strlen(ptr2)>0) {
				if (_stricmp(ptr2,"-value")==0) {
					ptr2 = strtok_int(NULL, '=',&saveptr2);
					if (ptr2) {
						myflagname=new char[strlen(ptr2)+1];strcpy_s(myflagname,strlen(ptr2)+1,ptr2);utils->StripQuot(&myflagname);
					} else {
						mesg->WriteNextLine(LOG_ERROR,CM_SYNTAX_ERROR,ptr,CurrentCommand);return com;
					}
				} else {
					mesg->WriteNextLine(LOG_ERROR,"Unknown option [%s] after [GUIRequestVersion]",ptr);return com;
				}
			} else {
				mesg->WriteNextLine(LOG_ERROR,CM_INVALID_OPTION,ptr,CurrentCommand);return com;
			}			
		}

		if (com == COM_FOCUSALL) {
			CurrentCommand = COM_FOCUSALL_CMD;
			if (ptr) {
				mesg->WriteNextLine(LOG_ERROR,CM_NO_OPTION_ALLOWED,ptr,CurrentCommand);return com;
			}
		}

		if (com == COM_TRIANGULATION) {
			CurrentCommand = COM_TRIANGULATION_CMD;
			if (ptr) {
				mesg->WriteNextLine(LOG_ERROR,CM_NO_OPTION_ALLOWED,ptr,CurrentCommand);return com;
			}
		}

		if (com == COM_REMOVEBROKENTRIANGLES) {
			CurrentCommand = COM_REMOVEBROKENTRIANGLES_CMD;
			if (ptr) {
				mesg->WriteNextLine(LOG_ERROR,CM_NO_OPTION_ALLOWED,ptr,CurrentCommand);return com;
			}
		}


		if (com == COM_FOCUSQUAD) {
			CurrentCommand = COM_FOCUSQUAD_CMD;
			ptr2 = strtok_int(ptr, '=',&saveptr2);
			if (ptr2!=NULL && strlen(ptr2)>0) {
				if (_stricmp(ptr2,"-x")==0) {
					ptr2 = strtok_int(NULL, '=',&saveptr2);
					if (ptr2)
						sscanf_s(ptr2,"%f",&quadx);
					else {
						mesg->WriteNextLine(LOG_ERROR,CM_SYNTAX_ERROR,ptr,CurrentCommand);return com;
					}
				} else if (_stricmp(ptr2,"-y")==0) {
					ptr2 = strtok_int(NULL, '=',&saveptr2);
					if (ptr2)
						sscanf_s(ptr2,"%f",&quady);
					else {
						mesg->WriteNextLine(LOG_ERROR,CM_SYNTAX_ERROR,ptr,CurrentCommand);return com;
					}
				} else {
					mesg->WriteNextLine(LOG_ERROR,CM_UNKNOWN_OPTION,ptr,CurrentCommand);return com;
				}
			} else {
				mesg->WriteNextLine(LOG_ERROR,CM_INVALID_OPTION,ptr,CurrentCommand);return com;
			}
		}

		if (com == COM_FOCUSREC) {
			CurrentCommand = COM_FOCUSREC_CMD;
			ptr2 = strtok_int(ptr, '=',&saveptr2);
			if (ptr2!=NULL && strlen(ptr2)>0) {
				if (_stricmp(ptr2,"-x1")==0) {
					ptr2 = strtok_int(NULL, '=',&saveptr2);
					if (ptr2) {
						sscanf_s(ptr2,"%f",&x00);}
					else {
						mesg->WriteNextLine(LOG_ERROR,CM_SYNTAX_ERROR,ptr,CurrentCommand);return com;
					}
				} else if (_stricmp(ptr2,"-y1")==0) {
					ptr2 = strtok_int(NULL, '=',&saveptr2);
					if (ptr2) {
						sscanf_s(ptr2,"%f",&y00);}
					else {
						mesg->WriteNextLine(LOG_ERROR,CM_SYNTAX_ERROR,ptr,CurrentCommand);return com;
					}
				} else if (_stricmp(ptr2,"-x2")==0) {
					ptr2 = strtok_int(NULL, '=',&saveptr2);
					if (ptr2) {
						sscanf_s(ptr2,"%f",&x11);}
					else {
						mesg->WriteNextLine(LOG_ERROR,CM_SYNTAX_ERROR,ptr,CurrentCommand);return com;
					}
				} else if (_stricmp(ptr2,"-y2")==0) {
					ptr2 = strtok_int(NULL, '=',&saveptr2);
					if (ptr2) {
						sscanf_s(ptr2,"%f",&y11);}
					else {
						mesg->WriteNextLine(LOG_ERROR,CM_SYNTAX_ERROR,ptr,CurrentCommand);return com;
					}
				} else {
					mesg->WriteNextLine(LOG_ERROR,CM_UNKNOWN_OPTION,ptr,CurrentCommand);return com;
				}
			} else {
				mesg->WriteNextLine(LOG_ERROR,CM_INVALID_OPTION,ptr,CurrentCommand);return com;
			}
		}

		if (com == COM_SETHEIGHT) {
			CurrentCommand = COM_SETHEIGHT_CMD;
			if (_stricmp(ptr,"-gameunits")==0) {
				usegameunits=1;
			} else {
				ptr2 = strtok_int(ptr, '=',&saveptr2);
				if (ptr2!=NULL && strlen(ptr2)>0) {
					if (_stricmp(ptr2,"-z")==0) {
						ptr2 = strtok_int(NULL, '=',&saveptr2);
						if (ptr2)
							sscanf_s(ptr2,"%f",&zmin);
						else {
							mesg->WriteNextLine(LOG_ERROR,CM_SYNTAX_ERROR,ptr,CurrentCommand);return com;
						}
					} else {
						mesg->WriteNextLine(LOG_ERROR,CM_UNKNOWN_OPTION,ptr,CurrentCommand);return com;
					}
				} else {
					mesg->WriteNextLine(LOG_ERROR,CM_INVALID_OPTION,ptr,CurrentCommand);return com;
				}
			}
		}

		if (com == COM_SETSINGLEPOINT) {
			CurrentCommand = COM_SETSINGLEPOINT_CMD;
			ptr2 = strtok_int(ptr, '=',&saveptr2);
			if (ptr2!=NULL && strlen(ptr2)>0) {
				if (_stricmp(ptr2,"-x")==0) {
					ptr2 = strtok_int(NULL, '=',&saveptr2);
					if (ptr2)
						sscanf_s(ptr2,"%f",&gridx);
					else {
						mesg->WriteNextLine(LOG_ERROR,CM_SYNTAX_ERROR,ptr,CurrentCommand);return com;
					}
				} else if (_stricmp(ptr2,"-y")==0) {
					ptr2 = strtok_int(NULL, '=',&saveptr2);
					if (ptr2)
						sscanf_s(ptr2,"%f",&gridy);
					else {
						mesg->WriteNextLine(LOG_ERROR,CM_SYNTAX_ERROR,ptr,CurrentCommand);return com;
					}
				} else {
					mesg->WriteNextLine(LOG_ERROR,CM_UNKNOWN_OPTION,ptr,CurrentCommand);return com;
				}
			} else {
				mesg->WriteNextLine(LOG_ERROR,CM_INVALID_OPTION,ptr,CurrentCommand);return com;
			}
		}

		if (com == COM_PANORAMA) {
			CurrentCommand = COM_PANORAMA_CMD;
			ptr2 = strtok_int(ptr, '=',&saveptr2);
			if (ptr2!=NULL && strlen(ptr2)>0) {
				if (_stricmp(ptr2,"-x")==0) {
					ptr2 = strtok_int(NULL, '=',&saveptr2);
					if (ptr2)
						sscanf_s(ptr2,"%f",&gridx);
					else {
						mesg->WriteNextLine(LOG_ERROR,CM_SYNTAX_ERROR,ptr,CurrentCommand);return com;
					}
				} else if (_stricmp(ptr2,"-y")==0) {
					ptr2 = strtok_int(NULL, '=',&saveptr2);
					if (ptr2)
						sscanf_s(ptr2,"%f",&gridy);
					else {
						mesg->WriteNextLine(LOG_ERROR,CM_SYNTAX_ERROR,ptr,CurrentCommand);return com;
					}
				} else if (_stricmp(ptr2,"-z")==0) {
					ptr2 = strtok_int(NULL, '=',&saveptr2);
					if (ptr2)
						sscanf_s(ptr2,"%f",&z);
					else {
						mesg->WriteNextLine(LOG_ERROR,CM_SYNTAX_ERROR,ptr,CurrentCommand);return com;
					}
				} else if (_stricmp(ptr2,"-keepout")==0) {
					ptr2 = strtok_int(NULL, '=',&saveptr2);
					if (ptr2)
						sscanf_s(ptr2,"%f",&Keepout);
					else {
						mesg->WriteNextLine(LOG_ERROR,CM_SYNTAX_ERROR,ptr,CurrentCommand);return com;
					}
				} else {
					mesg->WriteNextLine(LOG_ERROR,CM_UNKNOWN_OPTION,ptr,CurrentCommand);return com;
				}
			} else {
				mesg->WriteNextLine(LOG_ERROR,CM_INVALID_OPTION,ptr,CurrentCommand);return com;
			}
		}


		if (com == COM_WRITEALL) {
			CurrentCommand = COM_WRITEALL_CMD;
			if (_stricmp(ptr,"-writeheightmap")==0) {
				writeheightmap=1;
			} else if (_stricmp(ptr,"-writenormalmap")==0) {
				writenormalmap=1;
			} else {
				ptr2 = strtok_int(ptr, '=',&saveptr2);
				if (ptr2!=NULL && strlen(ptr2)>0) {
					if (strncmp(ptr2,"-texture",3)==0) {
						ptr2 = strtok_int(NULL, '=',&saveptr2);
						if (ptr2) {
							texname=new char[strlen(ptr2)+1];strcpy_s(texname,strlen(ptr2)+1,ptr2);utils->StripQuot(&texname);
						} else {
							mesg->WriteNextLine(LOG_ERROR,CM_SYNTAX_ERROR,ptr,CurrentCommand);return com;
						}
					} else if (_stricmp(ptr2,"-transx")==0) {
						ptr2 = strtok_int(NULL, '=',&saveptr2);
						if (ptr2)
							sscanf_s(ptr2,"%f",&trans_x);
						else {
							mesg->WriteNextLine(LOG_ERROR,CM_SYNTAX_ERROR,ptr,CurrentCommand);return com;
						}
					} else if (_stricmp(ptr2,"-transy")==0) {
						ptr2 = strtok_int(NULL, '=',&saveptr2);
						if (ptr2)
							sscanf_s(ptr2,"%f",&trans_y);
						else {
							mesg->WriteNextLine(LOG_ERROR,CM_SYNTAX_ERROR,ptr,CurrentCommand);return com;
						}
					} else if (_stricmp(ptr2,"-transz")==0) {
						ptr2 = strtok_int(NULL, '=',&saveptr2);
						if (ptr2)
							sscanf_s(ptr2,"%f",&trans_z);
						else {
							mesg->WriteNextLine(LOG_ERROR,CM_SYNTAX_ERROR,ptr,CurrentCommand);return com;
						}
					} else if (_stricmp(ptr2,"-name")==0) {
						ptr2 = strtok_int(NULL, '=',&saveptr2);
						if (ptr2) {
							optname=new char[strlen(ptr2)+1];strcpy_s(optname,strlen(ptr2)+1,ptr2);utils->StripQuot(&optname);
						} else {
							mesg->WriteNextLine(LOG_ERROR,CM_SYNTAX_ERROR,ptr,CurrentCommand);return com;
						}
					} else {
						mesg->WriteNextLine(LOG_ERROR,CM_UNKNOWN_OPTION,ptr,CurrentCommand);return com;
					}
				} else {
					mesg->WriteNextLine(LOG_ERROR,CM_INVALID_OPTION,ptr,CurrentCommand);return com;
				}
			} 
		}

		if (com == COM_READFILE) {
			CurrentCommand = COM_READFILE_CMD;
			ptr2 = strtok_int(ptr, '=',&saveptr2);
			if (ptr2!=NULL && strlen(ptr2)>0) {
				if (strncmp(ptr2,"-filename",3)==0) {
					ptr2 = strtok_int(NULL, '=',&saveptr2);
					if (ptr2) {
						datafile=new char[strlen(ptr2)+1];strcpy_s(datafile,strlen(ptr2)+1,ptr2);utils->StripQuot(&datafile);
					} else {
						mesg->WriteNextLine(LOG_ERROR,CM_SYNTAX_ERROR,ptr,CurrentCommand);return com;
					}
				}
			} else {
				mesg->WriteNextLine(LOG_ERROR,CM_INVALID_OPTION,ptr,CurrentCommand);return com;
			}
		}

		if (com == COM_READPOLYGONDATAFILE) {
			CurrentCommand = COM_READPOLYGONDATAFILE_CMD;
			ptr2 = strtok_int(ptr, '=',&saveptr2);
			if (ptr2!=NULL && strlen(ptr2)>0) {
				if (strncmp(ptr2,"-filename",4)==0) {
					ptr2 = strtok_int(NULL, '=',&saveptr2);
					if (ptr2) {
						datafile=new char[strlen(ptr2)+1];strcpy_s(datafile,strlen(ptr2)+1,ptr2);utils->StripQuot(&datafile);
					} else {
						mesg->WriteNextLine(LOG_ERROR,CM_SYNTAX_ERROR,ptr,CurrentCommand);return com;
					}
				} else if (strncmp(ptr2,"-name",4)==0) {
					ptr2 = strtok_int(NULL, '=',&saveptr2);
					if (ptr2) {
						polygon_name=new char[strlen(ptr2)+1];strcpy_s(polygon_name,strlen(ptr2)+1,ptr2);utils->StripQuot(&polygon_name);
					} else {
						mesg->WriteNextLine(LOG_ERROR,CM_SYNTAX_ERROR,ptr,CurrentCommand);return com;
					}
				} else {
					mesg->WriteNextLine(LOG_ERROR,CM_UNKNOWN_OPTION,ptr,CurrentCommand);return com;
				}
			} else {
				mesg->WriteNextLine(LOG_ERROR,"Unknown option [%s] after [ReadPolygonDataFile]",ptr);return com;
			}
		}

		if (com == COM_WRITEALLQUADS) {
			if (_stricmp(ptr,"-writeheightmap")==0) {
				writeheightmap=1;
			} else if (_stricmp(ptr,"-writenormalmap")==0) {
				writenormalmap=1;
			} else {
				mesg->WriteNextLine(LOG_ERROR,CM_INVALID_OPTION,ptr,CurrentCommand);return com;
			}
		}

		if (com == COM_WRITEQUAD) {
			CurrentCommand = COM_WRITEQUAD_CMD;
			if (_stricmp(ptr,"-ps")==0) {
				ps=1;
			} else if (_stricmp(ptr,"-writeheightmap")==0) {
				writeheightmap=1;
			} else if (_stricmp(ptr,"-writenormalmap")==0) {
				writenormalmap=1;
			} else {
				ptr2 = strtok_int(ptr, '=',&saveptr2);
				if (ptr2!=NULL && strlen(ptr2)>0) {
					if (_stricmp(ptr2,"-x")==0) {
						ptr2 = strtok_int(NULL, '=',&saveptr2);
						if (ptr2)
							sscanf_s(ptr2,"%f",&quadx);
						else {
							mesg->WriteNextLine(LOG_ERROR,CM_SYNTAX_ERROR,ptr,CurrentCommand);return com;
						}
					} else if (_stricmp(ptr2,"-y")==0) {
						ptr2 = strtok_int(NULL, '=',&saveptr2);
						if (ptr2)
							sscanf_s(ptr2,"%f",&quady);
						else {
							mesg->WriteNextLine(LOG_ERROR,CM_SYNTAX_ERROR,ptr,CurrentCommand);return com;
						}
					} else if (strncmp(ptr2,"-texture",3)==0) {
						ptr2 = strtok_int(NULL, '=',&saveptr2);
						if (ptr2) {
							texname=new char[strlen(ptr2)+1];strcpy_s(texname,strlen(ptr2)+1,ptr2);utils->StripQuot(&texname);
						} else {
							mesg->WriteNextLine(LOG_ERROR,CM_SYNTAX_ERROR,ptr,CurrentCommand);return com;
						}
					} else {
						mesg->WriteNextLine(LOG_ERROR,CM_UNKNOWN_OPTION,ptr,CurrentCommand);return com;
					}
				} else {
					mesg->WriteNextLine(LOG_ERROR,CM_INVALID_OPTION,ptr,CurrentCommand);return com;
				}
			}
		}

		if (com == COM_SETGRID) {
			CurrentCommand = COM_SETGRID_CMD;
			ptr2 = strtok_int(ptr, '=',&saveptr2);
			if (ptr2!=NULL && strlen(ptr2)>0) {
				if (_stricmp(ptr2,"-x")==0) {
					ptr2 = strtok_int(NULL, '=',&saveptr2);
					if (ptr2)
						sscanf_s(ptr2,"%f",&gridx);
					else {
						mesg->WriteNextLine(LOG_ERROR,CM_SYNTAX_ERROR,ptr,CurrentCommand);return com;
					}
				} else if (_stricmp(ptr2,"-y")==0) {
					ptr2 = strtok_int(NULL, '=',&saveptr2);
					if (ptr2)
						sscanf_s(ptr2,"%f",&gridy);
					else {
						mesg->WriteNextLine(LOG_ERROR,CM_SYNTAX_ERROR,ptr,CurrentCommand);return com;
					}
				}
			} else {
				mesg->WriteNextLine(LOG_ERROR,CM_INVALID_OPTION,ptr,CurrentCommand);return com;
			}
		}

		if (com == COM_SETGRIDBORDER) {
			CurrentCommand = COM_SETGRIDBORDER_CMD;
			ptr2 = strtok_int(ptr, '=',&saveptr2);
			if (ptr2!=NULL && strlen(ptr2)>0) {
				if (_stricmp(ptr2,"-x")==0) {
					ptr2 = strtok_int(NULL, '=',&saveptr2);
					if (ptr2)
						sscanf_s(ptr2,"%f",&gridx);
					else { 
						mesg->WriteNextLine(LOG_ERROR,CM_SYNTAX_ERROR,ptr,CurrentCommand);return com;
					}
				} else if (_stricmp(ptr2,"-y")==0) {
					ptr2 = strtok_int(NULL, '=',&saveptr2);
					if (ptr2)
						sscanf_s(ptr2,"%f",&gridy);
					else {
						mesg->WriteNextLine(LOG_ERROR,CM_SYNTAX_ERROR,ptr,CurrentCommand);return com;
					}
				} else if (_stricmp(ptr2,"-min")==0) {
					ptr2 = strtok_int(NULL, '=',&saveptr2);
					if (ptr2)
						sscanf_s(ptr2,"%f",&zz1);
					else {
						mesg->WriteNextLine(LOG_ERROR,CM_SYNTAX_ERROR,ptr,CurrentCommand);return com;
					}
				} else {
					mesg->WriteNextLine(LOG_ERROR,CM_UNKNOWN_OPTION,ptr,CurrentCommand);return com;
				}
			} else {
				mesg->WriteNextLine(LOG_ERROR,CM_INVALID_OPTION,ptr,CurrentCommand);return com;
			}
		}

		if (com == COM_BREAKATGRID) {
			CurrentCommand = COM_BREAKATGRID_CMD;
			ptr2 = strtok_int(ptr, '=',&saveptr2);
			if (ptr2!=NULL && strlen(ptr2)>0) {
				if (_stricmp(ptr2,"-x")==0) {
					ptr2 = strtok_int(NULL, '=',&saveptr2);
					if (ptr2)
						sscanf_s(ptr2,"%f",&gridx);
					else {
						mesg->WriteNextLine(LOG_ERROR,CM_SYNTAX_ERROR,ptr,CurrentCommand);return com;
					}
				} else if (_stricmp(ptr2,"-y")==0) {
					ptr2 = strtok_int(NULL, '=',&saveptr2);
					if (ptr2)
						sscanf_s(ptr2,"%f",&gridy);
					else {
						mesg->WriteNextLine(LOG_ERROR,CM_SYNTAX_ERROR,ptr,CurrentCommand);return com;
					}
				} else if (_stricmp(ptr2,"-max")==0) {
					ptr2 = strtok_int(NULL, '=',&saveptr2);
					if (ptr2)
						sscanf_s(ptr2,"%f",&max);
					else {
						mesg->WriteNextLine(LOG_ERROR,CM_SYNTAX_ERROR,ptr,CurrentCommand);return com;
					}
				} else if (_stricmp(ptr2,"-zmin")==0) {
					ptr2 = strtok_int(NULL, '=',&saveptr2);
					if (ptr2)
						sscanf_s(ptr2,"%f",&zmin);
					else {
						mesg->WriteNextLine(LOG_ERROR,CM_SYNTAX_ERROR,ptr,CurrentCommand);return com;
					}
				} else {
					mesg->WriteNextLine(LOG_ERROR,CM_UNKNOWN_OPTION,ptr,CurrentCommand);return com;
				}
			} else {
				mesg->WriteNextLine(LOG_ERROR,CM_INVALID_OPTION,ptr,CurrentCommand);return com;
			}
		}

		if (com == COM_BREAKFLATTRIANGLES) {
			CurrentCommand = COM_BREAKFLATTRIANGLES_CMD;
			ptr2 = strtok_int(ptr, '=',&saveptr2);
			if (ptr2!=NULL && strlen(ptr2)>0) {
				if (strncmp(ptr2,"-highest",5)==0) {
					ptr2 = strtok_int(NULL, '=',&saveptr2);
					if (ptr2)
						sscanf_s(ptr2,"%f",&Highest);
					else {
						mesg->WriteNextLine(LOG_ERROR,CM_SYNTAX_ERROR,ptr,CurrentCommand);return com;
					}
				} else if (strncmp(ptr2,"-lowest",4)==0) {
					ptr2 = strtok_int(NULL, '=',&saveptr2);
					if (ptr2)
						sscanf_s(ptr2,"%f",&Lowest);
					else {
						mesg->WriteNextLine(LOG_ERROR,CM_SYNTAX_ERROR,ptr,CurrentCommand);return com;
					}
				} else if (strncmp(ptr2,"-z",2)==0) {
					ptr2 = strtok_int(NULL, '=',&saveptr2);
					if (ptr2)
						sscanf_s(ptr2,"%f",&z);
					else {
						mesg->WriteNextLine(LOG_ERROR,CM_SYNTAX_ERROR,ptr,CurrentCommand);return com;
					}
				} else {
					mesg->WriteNextLine(LOG_ERROR,CM_UNKNOWN_OPTION,ptr,CurrentCommand);return com;
				}
			} else {
				mesg->WriteNextLine(LOG_ERROR,CM_INVALID_OPTION,ptr,CurrentCommand);return com;
			}
		}

		if (com == COM_BREAKLINE) {
			CurrentCommand = COM_BREAKLINE_CMD;
			if (_stricmp(ptr,"-setmin")==0) {
				setmin=1;
			} else if (_stricmp(ptr,"-setmax")==0) {
				setmax=1;
			} else if (_stricmp(ptr,"-findmin")==0) {
				findmin=1;
			} else if (_stricmp(ptr,"-findmax")==0) {
				findmax=1;
			} else if (_stricmp(ptr,"-onlyintracell")==0) {
				onlyintracell=1;
			} else {
				ptr2 = strtok_int(ptr, '=',&saveptr2);
				if (ptr2!=NULL && strlen(ptr2)>0) {
				    if (_stricmp(ptr2,"-x")==0) {
						ptr2 = strtok_int(NULL, '=',&saveptr2);
						if (ptr2)
							sscanf_s(ptr2,"%f",&gridx);
						else {
							mesg->WriteNextLine(LOG_ERROR,CM_SYNTAX_ERROR,ptr,CurrentCommand);return com;
						}
					} else if (_stricmp(ptr2,"-y")==0) {
						ptr2 = strtok_int(NULL, '=',&saveptr2);
						if (ptr2)
							sscanf_s(ptr2,"%f",&gridy);
						else {
							mesg->WriteNextLine(LOG_ERROR,CM_SYNTAX_ERROR,ptr,CurrentCommand);return com;
						}
					} else if (_stricmp(ptr2,"-z")==0) {
						ptr2 = strtok_int(NULL, '=',&saveptr2);
						if (ptr2)
							sscanf_s(ptr2,"%f",&z);
						else {
							mesg->WriteNextLine(LOG_ERROR,CM_SYNTAX_ERROR,ptr,CurrentCommand);return com;
						}
					} else if (_stricmp(ptr2,"-offsetx")==0) {
						ptr2 = strtok_int(NULL, '=',&saveptr2);
						if (ptr2)
							sscanf_s(ptr2,"%f",&offsetx);
						else {
							mesg->WriteNextLine(LOG_ERROR,CM_SYNTAX_ERROR,ptr,CurrentCommand);return com;
						}
					} else if (_stricmp(ptr2,"-offsety")==0) {
						ptr2 = strtok_int(NULL, '=',&saveptr2);
						if (ptr2)
							sscanf_s(ptr2,"%f",&offsety);
						else {
							mesg->WriteNextLine(LOG_ERROR,CM_SYNTAX_ERROR,ptr,CurrentCommand);return com;}
					} else {mesg->WriteNextLine(LOG_ERROR,CM_UNKNOWN_OPTION,ptr,CurrentCommand);return com;}
				} else {mesg->WriteNextLine(LOG_ERROR,CM_INVALID_OPTION,ptr,CurrentCommand);return com;}
			}
		}

		if (com == COM_DIVIDEGRID) {
			CurrentCommand = COM_DIVIDEGRID_CMD;
			ptr2 = strtok_int(ptr, '=',&saveptr2);
			if (ptr2!=NULL && strlen(ptr2)>0) {
				if (_stricmp(ptr2,"-x")==0) {
					ptr2 = strtok_int(NULL, '=',&saveptr2);
					if (ptr2)
						sscanf_s(ptr2,"%f",&gridx);
					else {
						mesg->WriteNextLine(LOG_ERROR,CM_SYNTAX_ERROR,ptr,CurrentCommand);return com;
					}
				} else if (_stricmp(ptr2,"-y")==0) {
					ptr2 = strtok_int(NULL, '=',&saveptr2);
					if (ptr2)
						sscanf_s(ptr2,"%f",&gridy);
					else {
						mesg->WriteNextLine(LOG_ERROR,CM_SYNTAX_ERROR,ptr,CurrentCommand);return com;
					}
				} else {
					mesg->WriteNextLine(LOG_ERROR,CM_UNKNOWN_OPTION,ptr,CurrentCommand);return com;
				}
			} else {
				mesg->WriteNextLine(LOG_ERROR,"Unknown option [%s] after [%s]",ptr,CurrentCommand);return com;
			}
		}

		if (com == COM_DIVIDEAT) {
			CurrentCommand = COM_DIVIDEAT_CMD;
			ptr2 = strtok_int(ptr, '=',&saveptr2);
			if (ptr2!=NULL && strlen(ptr2)>0) {
				if (_stricmp(ptr2,"-x")==0) {
					ptr2 = strtok_int(NULL, '=',&saveptr2);
					if (ptr2)
						sscanf_s(ptr2,"%f",&gridx);
					else {
						mesg->WriteNextLine(LOG_ERROR,CM_SYNTAX_ERROR,ptr,CurrentCommand);return com;
					}
				} else if (_stricmp(ptr2,"-y")==0) {
					ptr2 = strtok_int(NULL, '=',&saveptr2);
					if (ptr2)
						sscanf_s(ptr2,"%f",&gridy);
					else {
						mesg->WriteNextLine(LOG_ERROR,CM_SYNTAX_ERROR,ptr,CurrentCommand);return com;
					}
				} else {
					mesg->WriteNextLine(LOG_ERROR,CM_UNKNOWN_OPTION,ptr,CurrentCommand);return com;
				}
			} else {
				mesg->WriteNextLine(LOG_ERROR,CM_INVALID_OPTION,ptr,CurrentCommand);return com;
			}
		}

		if (com == COM_DIVIDEBETWEEN) {
			CurrentCommand = COM_DIVIDEBETWEEN_CMD;
			ptr2 = strtok_int(ptr, '=',&saveptr2);
			if (ptr2!=NULL && strlen(ptr2)>0) {
				if (_stricmp(ptr2,"-x1")==0) {
					ptr2 = strtok_int(NULL, '=',&saveptr2);
					if (ptr2)
						sscanf_s(ptr2,"%f",&xx1);
					else {
						mesg->WriteNextLine(LOG_ERROR,CM_SYNTAX_ERROR,ptr,CurrentCommand);return com;
					}
				} else if (_stricmp(ptr2,"-y1")==0) {
					ptr2 = strtok_int(NULL, '=',&saveptr2);
					if (ptr2)
						sscanf_s(ptr2,"%f",&yy1);
					else {
						mesg->WriteNextLine(LOG_ERROR,CM_SYNTAX_ERROR,ptr,CurrentCommand);return com;
					}
				} else if (_stricmp(ptr2,"-x2")==0) {
					ptr2 = strtok_int(NULL, '=',&saveptr2);
					if (ptr2)
						sscanf_s(ptr2,"%f",&xx2);
					else {
						mesg->WriteNextLine(LOG_ERROR,CM_SYNTAX_ERROR,ptr,CurrentCommand);return com;
					}
				} else if (_stricmp(ptr2,"-y2")==0) {
					ptr2 = strtok_int(NULL, '=',&saveptr2);
					if (ptr2)
						sscanf_s(ptr2,"%f",&yy2);
					else {
						mesg->WriteNextLine(LOG_ERROR,CM_SYNTAX_ERROR,ptr,CurrentCommand);return com;
					}
				} else {
					mesg->WriteNextLine(LOG_ERROR,CM_UNKNOWN_OPTION,ptr,CurrentCommand);return com;
				}
			} else {
				mesg->WriteNextLine(LOG_ERROR,CM_INVALID_OPTION,ptr,CurrentCommand);return com;
			}
		}

		if (com == COM_CREATEPOLYGON) {
			CurrentCommand = COM_CREATEPOLYGON_CMD;
			ptr2 = strtok_int(ptr, '=',&saveptr2);
			if (ptr2!=NULL && strlen(ptr2)>0) {
				if (_stricmp(ptr2,"-x1")==0) {
					ptr2 = strtok_int(NULL, '=',&saveptr2);
					if (ptr2)
						sscanf_s(ptr2,"%f",&xx1);
					else {
						mesg->WriteNextLine(LOG_ERROR,CM_SYNTAX_ERROR,ptr,CurrentCommand);return com;
					}
				} else if (_stricmp(ptr2,"-y1")==0) {
					ptr2 = strtok_int(NULL, '=',&saveptr2);
					if (ptr2)
						sscanf_s(ptr2,"%f",&yy1);
					else {
						mesg->WriteNextLine(LOG_ERROR,CM_SYNTAX_ERROR,ptr,CurrentCommand);return com;
					}
				} else if (_stricmp(ptr2,"-x2")==0) {
					ptr2 = strtok_int(NULL, '=',&saveptr2);
					if (ptr2)
						sscanf_s(ptr2,"%f",&xx2);
					else {
						mesg->WriteNextLine(LOG_ERROR,CM_SYNTAX_ERROR,ptr,CurrentCommand);return com;
					}
				} else if (_stricmp(ptr2,"-y2")==0) {
					ptr2 = strtok_int(NULL, '=',&saveptr2);
					if (ptr2)
						sscanf_s(ptr2,"%f",&yy2);
					else {
						mesg->WriteNextLine(LOG_ERROR,CM_SYNTAX_ERROR,ptr,CurrentCommand);return com;
					}
				} else if (_stricmp(ptr2,"-name")==0) {
					ptr2 = strtok_int(NULL, '=',&saveptr2);
					if (ptr2) {
						polygon_name=new char[strlen(ptr2)+1];strcpy_s(polygon_name,strlen(ptr2)+1,ptr2);utils->StripQuot(&polygon_name);
					} else {
						mesg->WriteNextLine(LOG_ERROR,CM_SYNTAX_ERROR,ptr,CurrentCommand);return com;
					}
				} else {
					mesg->WriteNextLine(LOG_ERROR,CM_UNKNOWN_OPTION,ptr,CurrentCommand);return com;
				}
			} else {
				mesg->WriteNextLine(LOG_ERROR,CM_INVALID_OPTION,ptr,CurrentCommand);return com;
			}
		}

		if (com == COM_ADDVERTEXTOPOLYGON) {
			CurrentCommand = COM_ADDVERTEXTOPOLYGON_CMD;
			ptr2 = strtok_int(ptr, '=',&saveptr2);
			if (ptr2!=NULL && strlen(ptr2)>0) {
				if (_stricmp(ptr2,"-x")==0) {
					ptr2 = strtok_int(NULL, '=',&saveptr2);
					if (ptr2)
						sscanf_s(ptr2,"%f",&xx1);
					else {
						mesg->WriteNextLine(LOG_ERROR,CM_SYNTAX_ERROR,ptr,CurrentCommand);return com;
					}
				}
				else if (_stricmp(ptr2,"-y")==0) {
					ptr2 = strtok_int(NULL, '=',&saveptr2);
					if (ptr2)
						sscanf_s(ptr2,"%f",&yy1);
					else {
						mesg->WriteNextLine(LOG_ERROR,CM_SYNTAX_ERROR,ptr,CurrentCommand);return com;
					}
				} else if (_stricmp(ptr2,"-name")==0) {
					ptr2 = strtok_int(NULL, '=',&saveptr2);
					if (ptr2) {
						polygon_name=new char[strlen(ptr2)+1];strcpy_s(polygon_name,strlen(ptr2)+1,ptr2);utils->StripQuot(&polygon_name);
					} else {
						mesg->WriteNextLine(LOG_ERROR,CM_SYNTAX_ERROR,ptr,CurrentCommand);return com;
					}
				} else {
					mesg->WriteNextLine(LOG_ERROR,"Unknown option [%s] after [AddVertexToPolygon]",ptr);return com;
				}
			} 
		}
		
		if (com == COM_DIVIDEATPOLGONBORDER) {
			CurrentCommand = COM_DIVIDEATPOLGONBORDER_CMD;
			ptr2 = strtok_int(ptr, '=',&saveptr2);
			if (ptr2!=NULL && strlen(ptr2)>0) {
				if (_stricmp(ptr2,"-name")==0) {
					ptr2 = strtok_int(NULL, '=',&saveptr2);
					if (ptr2) {
						polygon_name=new char[strlen(ptr2)+1];strcpy_s(polygon_name,strlen(ptr2)+1,ptr2);utils->StripQuot(&polygon_name);
					} else {
						mesg->WriteNextLine(LOG_ERROR,CM_SYNTAX_ERROR,ptr,CurrentCommand);return com;
					}
				} else {
					mesg->WriteNextLine(LOG_ERROR,CM_UNKNOWN_OPTION,ptr,CurrentCommand);return com;
				}
			} else {
				mesg->WriteNextLine(LOG_ERROR,CM_INVALID_OPTION,ptr,CurrentCommand);return com;
			}
		}

		if (com == COM_STENCILPOLGON) {
			CurrentCommand = COM_STENCILPOLGON_CMD;
			ptr2 = strtok_int(ptr, '=',&saveptr2);
			if (ptr2!=NULL && strlen(ptr2)>0) {
				if (_stricmp(ptr2,"-name")==0) {
					ptr2 = strtok_int(NULL, '=',&saveptr2);
					if (ptr2) {
						polygon_name=new char[strlen(ptr2)+1];strcpy_s(polygon_name,strlen(ptr2)+1,ptr2);utils->StripQuot(&polygon_name);
					} else {
						mesg->WriteNextLine(LOG_ERROR,CM_SYNTAX_ERROR,ptr,CurrentCommand);return com;
					}
				} else {
					mesg->WriteNextLine(LOG_ERROR,"Unknown option [%s] after [StencilPolygon]",ptr);return com;
				}
			} 
		}

		if (com == COM_INACTIVATEALLVERTICES) {
			CurrentCommand = COM_INACTIVATEALLVERTICES_CMD;
			if (ptr) {
				mesg->WriteNextLine(LOG_ERROR,CM_NO_OPTION_ALLOWED,ptr,CurrentCommand);return com;
			}
		}

		if (com == COM_REMOVEINACTIVETRIANGLES) {
			CurrentCommand = COM_REMOVEINACTIVETRIANGLES_CMD;
			if (ptr) {
				mesg->WriteNextLine(LOG_ERROR,CM_NO_OPTION_ALLOWED,ptr,CurrentCommand);return com;
			}
		}

		if (com == COM_ACTIVATEVISIBLEVERTICES) {
			CurrentCommand = COM_ACTIVATEVISIBLEVERTICES_CMD;
			ptr2 = strtok_int(ptr, '=',&saveptr2);
			if (ptr2!=NULL && strlen(ptr2)>0) {
				if (_stricmp(ptr2,"-x")==0) {
					ptr2 = strtok_int(NULL, '=',&saveptr2);
					if (ptr2)
						sscanf_s(ptr2,"%f",&xx1);
					else {
						mesg->WriteNextLine(LOG_ERROR,CM_SYNTAX_ERROR,ptr,CurrentCommand);return com;
					}
				} else if (_stricmp(ptr2,"-y")==0) {
					ptr2 = strtok_int(NULL, '=',&saveptr2);
					if (ptr2)
						sscanf_s(ptr2,"%f",&yy1);
					else {
						mesg->WriteNextLine(LOG_ERROR,CM_SYNTAX_ERROR,ptr,CurrentCommand);return com;
					}
				} else if (_stricmp(ptr2,"-z")==0) {
					ptr2 = strtok_int(NULL, '=',&saveptr2);
					if (ptr2)
						sscanf_s(ptr2,"%f",&zz1);
					else {
						mesg->WriteNextLine(LOG_ERROR,CM_SYNTAX_ERROR,ptr,CurrentCommand);return com;
					}
				} else if (_stricmp(ptr2,"-radius")==0) {
					ptr2 = strtok_int(NULL, '=',&saveptr2);
					if (ptr2)
						sscanf_s(ptr2,"%f",&OptRadius);
					else {
						mesg->WriteNextLine(LOG_ERROR,CM_SYNTAX_ERROR,ptr,CurrentCommand);return com;
					}
				} else {
					mesg->WriteNextLine(LOG_ERROR,CM_UNKNOWN_OPTION,ptr,CurrentCommand);return com;
				}
			} else {
				mesg->WriteNextLine(LOG_ERROR,CM_INVALID_OPTION,ptr,CurrentCommand);return com;
			}
		}


		if (com == COM_SETPOINTS || com == COM_SETPOINTSPERQUAD || 
			com == COM_SETMAXPOINTS || com == COM_SETMAXPOINTSPERQUAD) {
				ptr2 = strtok_int(ptr, '=',&saveptr2);
				if (ptr2!=NULL && strlen(ptr2)>0) {
					if (_stricmp(ptr2,"-n")==0) {
						ptr2 = strtok_int(NULL, '=',&saveptr2);
						if (ptr2)
							sscanf_s(ptr2,"%i",&npoints);
						else {
							mesg->WriteNextLine(LOG_ERROR,CM_SYNTAX_ERROR,ptr,CurrentCommand);return com;
						}
					} else {
						mesg->WriteNextLine(LOG_ERROR,CM_UNKNOWN_OPTION,ptr,CurrentCommand);return com;
					}
				} else {
					mesg->WriteNextLine(LOG_ERROR,CM_INVALID_OPTION,ptr,CurrentCommand);return com;
				}
		}

		if (com == COM_READBMP) {
			CurrentCommand = COM_READBMP_CMD;
			ptr2 = strtok_int(ptr, '=',&saveptr2);
			if (ptr2!=NULL && strlen(ptr2)>0) {
				if (_stricmp(ptr2,"-n")==0) {
					ptr2 = strtok_int(NULL, '=',&saveptr2);
					if (ptr2)
						sscanf_s(ptr2,"%i",&npoints);
					else {
						mesg->WriteNextLine(LOG_ERROR,CM_SYNTAX_ERROR,ptr,CurrentCommand);return com;
					}
				} else {
					mesg->WriteNextLine(LOG_ERROR,CM_UNKNOWN_OPTION,ptr,CurrentCommand);return com;
				}
			} else {
				mesg->WriteNextLine(LOG_ERROR,CM_INVALID_OPTION,ptr,CurrentCommand);return com;
			}
		}

		if (com == COM_GENERATEHEIGHTMAP) {
			CurrentCommand = COM_GENERATEHEIGHTMAP_CMD;
			if (_stricmp(ptr,"-quick")==0) {
				quick=1;
			} else {
				ptr2 = strtok_int(ptr, '=',&saveptr2);
				if (ptr2!=NULL && strlen(ptr2)>0) {
					if (_stricmp(ptr2,"-n")==0) {
						ptr2 = strtok_int(NULL, '=',&saveptr2);
						if (ptr2)
							sscanf_s(ptr2,"%i",&npoints);
						else {
							mesg->WriteNextLine(LOG_ERROR,CM_SYNTAX_ERROR,ptr,CurrentCommand);return com;
						}
					} else {
						mesg->WriteNextLine(LOG_ERROR,CM_UNKNOWN_OPTION,ptr,CurrentCommand);return com;
					}
				} 
			}
		}

		if (com == COM_FILTER) {
			CurrentCommand = COM_FILTER_CMD;
			if (_stricmp(ptr,"-overwrite")==0) {
				overwrite=1;
			} else {
				ptr2 = strtok_int(ptr, '=',&saveptr2);
				if (ptr2!=NULL && strlen(ptr2)>0) {
					if (_stricmp(ptr2,"-n")==0) {
						ptr2 = strtok_int(NULL, '=',&saveptr2);
						if (ptr2)
							sscanf_s(ptr2,"%i",&npoints);
						else {
							mesg->WriteNextLine(LOG_ERROR,CM_SYNTAX_ERROR,ptr,CurrentCommand);return com;
						}
					} else {
						mesg->WriteNextLine(LOG_ERROR,CM_UNKNOWN_OPTION,ptr,CurrentCommand);return com;
					}
				} else {
					mesg->WriteNextLine(LOG_ERROR,CM_INVALID_OPTION,ptr,CurrentCommand);return com;
				}
			}
		}

		if (com == COM_CALLTES4QLOD) {
			CurrentCommand = COM_CALLTES4QLOD_CMD;
			if (_stricmp(ptr,"-silent")==0) {
				tes4qlod_silent=1;
			} else {
				ptr2 = strtok_int(ptr, '=',&saveptr2);
				if (ptr2!=NULL && strlen(ptr2)>0) {
					if (_stricmp(ptr2,"-q")==0) {
						ptr2 = strtok_int(NULL, '=',&saveptr2);
						if (ptr2)
							sscanf_s(ptr2,"%i",&tes4qlod_q);
						else {
							mesg->WriteNextLine(LOG_ERROR,CM_SYNTAX_ERROR,ptr,CurrentCommand);return com;
						}
					} else if (_stricmp(ptr2,"-options")==0) {
						ptr2 = strtok_int(NULL, '=',&saveptr2);
						if (ptr2) {
							tes4qlod_options=new char[strlen(ptr2)+1];strcpy_s(tes4qlod_options,strlen(ptr2)+1,ptr2);utils->StripQuot(&tes4qlod_options);
						} else {
							mesg->WriteNextLine(LOG_ERROR,CM_SYNTAX_ERROR,ptr,CurrentCommand);return com;
						}
					} else {
						mesg->WriteNextLine(LOG_ERROR,CM_UNKNOWN_OPTION,ptr,CurrentCommand);return com;
					}
				} 
			}
		}

		if (com == COM_SETOPTION) {
			CurrentCommand = COM_SETOPTION_CMD;
			if (_stricmp(ptr,"-createpedestals")==0) {
				mesg->WriteNextLine(LOG_COMMAND,"SetOption -createpedestals");
				createpedestals=1;
			} else if (_stricmp(ptr,"-useshapes")==0) {
				mesg->WriteNextLine(LOG_COMMAND,"SetOption -useshapes");
				useshapes=1;
			} else if (_stricmp(ptr,"-use16bit")==0) {
				mesg->WriteNextLine(LOG_COMMAND,"SetOption -use16bit");
				use16bit=1;
			} else if (_stricmp(ptr,"-lodshadows")==0) {
				mesg->WriteNextLine(LOG_COMMAND,"SetOption -lodshadows");
				lodshadows=1;
			} else if (_stricmp(ptr,"-gamemodeoblivion")==0) {
				mesg->WriteNextLine(LOG_COMMAND,"SetOption -gamemodeoblivion");
				gamemode=2;
			} else if (_stricmp(ptr,"-gamemodeskyrim")==0) {
				mesg->WriteNextLine(LOG_COMMAND,"SetOption -gamemodeskyrim");
				gamemode=5;
			} else if (_stricmp(ptr,"-noskipinfo")==0) {		
				mesg->WriteNextLine(LOG_COMMAND,"SetOption -noskipinfo");
				noskipinfo=1;
			} else if (_stricmp(ptr,"-center")==0) {		
				mesg->WriteNextLine(LOG_COMMAND,"SetOption -center");
				center=1;
			} else {
				ptr2 = strtok_int(ptr, '=',&saveptr2);
				if (ptr2!=NULL && strlen(ptr2)>0) {
					if (_stricmp(ptr2,"-mindistance")==0) {
						ptr2 = strtok_int(NULL, '=',&saveptr2);
						utils->StripQuot(&ptr2);
						if (ptr2)
							sscanf_s(ptr2,"%i",&mindistance);
						else {
							mesg->WriteNextLine(LOG_ERROR,CM_SYNTAX_ERROR,ptr,CurrentCommand);return com;
						}
					} else if (_stricmp(ptr2,"-zboost")==0) {
						ptr2 = strtok_int(NULL, '=',&saveptr2);
						utils->StripQuot(&ptr2);
						if (ptr2)
							sscanf_s(ptr2,"%f",&overdrawing);
						else {
							mesg->WriteNextLine(LOG_ERROR,CM_SYNTAX_ERROR,ptr,CurrentCommand);return com;
						}
					} else if (_stricmp(ptr2,"-nquadmax")==0) {
						ptr2 = strtok_int(NULL, '=',&saveptr2);
						utils->StripQuot(&ptr2);
						if (ptr2)
							sscanf_s(ptr2,"%i",&nquadmax);
						else {
							mesg->WriteNextLine(LOG_ERROR,CM_SYNTAX_ERROR,ptr,CurrentCommand);return com;
						}
					} else if (_stricmp(ptr2,"-sizex")==0) {
						ptr2 = strtok_int(NULL, '=',&saveptr2);
						utils->StripQuot(&ptr2);
						if (ptr2)
							sscanf_s(ptr2,"%i",&size_x);
						else {
							mesg->WriteNextLine(LOG_ERROR,CM_SYNTAX_ERROR,ptr,CurrentCommand);return com;
						}
					} else if (_stricmp(ptr2,"-sizey")==0) {
						ptr2 = strtok_int(NULL, '=',&saveptr2);
						utils->StripQuot(&ptr2);
						if (ptr2)
							sscanf_s(ptr2,"%i",&size_y);
						else {
							mesg->WriteNextLine(LOG_ERROR,CM_SYNTAX_ERROR,ptr,CurrentCommand);return com;
						}
					} else if (_stricmp(ptr2,"-worldspaceid")==0) {
						ptr2 = strtok_int(NULL, '=',&saveptr2);
						utils->StripQuot(&ptr2);
						if (ptr2) {
							mesg->WriteNextLine(LOG_INFO,"SetOption -worldspaceid=%s", ptr2);
							worldname=new char[strlen(ptr2)+1];strcpy_s(worldname,strlen(ptr2)+1,ptr2);utils->StripQuot(&worldname);
						} else {
							mesg->WriteNextLine(LOG_ERROR,CM_SYNTAX_ERROR,ptr,CurrentCommand);return com;
						}
					} else if (_stricmp(ptr2,"-installdirectory")==0) {					
						ptr2 = strtok_int(NULL, '=',&saveptr2);
						utils->StripQuot(&ptr2);
						if (ptr2) {
							mesg->WriteNextLine(LOG_INFO,"SetOption -installdirectory=%s", ptr2);
							install_dir=new char[strlen(ptr2)+1];strcpy_s(install_dir,strlen(ptr2)+1,ptr2);utils->StripQuot(&install_dir);
							if (strlen(install_dir)>0) {
								for (unsigned int i=0;i<strlen(install_dir);i++) {
									if (install_dir[i]=='\\') {
										install_dir[i]='\0';
										_mkdir(install_dir);
										install_dir[i]='\\';
									}
								}
								_mkdir(install_dir);
							}
						} else {
							mesg->WriteNextLine(LOG_ERROR,CM_SYNTAX_ERROR,ptr,CurrentCommand);return com;
						}
					} else if (_stricmp(ptr2,"-ddstool")==0) {					
						ptr2 = strtok_int(NULL, '=',&saveptr2);
						utils->StripQuot(&ptr2);
						if (ptr2) {
							mesg->WriteNextLine(LOG_INFO,"SetOption -ddstool=%s", ptr2);
							dds_tool=new char[strlen(ptr2)+1];strcpy_s(dds_tool,strlen(ptr2)+1,ptr2);utils->StripQuot(&dds_tool);
						} else {
							mesg->WriteNextLine(LOG_ERROR,CM_SYNTAX_ERROR,ptr,CurrentCommand);return com;
						}
					} else if (_stricmp(ptr2,"-minheight")==0) {					
						ptr2 = strtok_int(NULL, '=',&saveptr2);
						utils->StripQuot(&ptr2);
						if (ptr2) {
							mesg->WriteNextLine(LOG_INFO,"SetOption -minheight=%s", ptr2);
							sscanf_s(ptr2,"%f",&minheight);
						} else {
							mesg->WriteNextLine(LOG_ERROR,CM_SYNTAX_ERROR,ptr,CurrentCommand);return com;
						}
					} else if (_stricmp(ptr2,"-bmpscaling")==0) {					
						ptr2 = strtok_int(NULL, '=',&saveptr2);
						utils->StripQuot(&ptr2);
						if (ptr2) {
							mesg->WriteNextLine(LOG_INFO,"SetOption -bmpscaling=%s", ptr2);
							sscanf_s(ptr2,"%f",&bmpscaling);
						} else {
							mesg->WriteNextLine(LOG_ERROR,CM_SYNTAX_ERROR,ptr,CurrentCommand);return com;
						}
					} else if (_stricmp(ptr2,"-quadtreelevels")==0) {					
						ptr2 = strtok_int(NULL, '=',&saveptr2);
						utils->StripQuot(&ptr2);
						if (ptr2) {
							sscanf_s(ptr2,"%f",&quadtreelevels);
						} else {
							mesg->WriteNextLine(LOG_ERROR,CM_SYNTAX_ERROR,ptr,CurrentCommand);return com;
						}
					} else {
						mesg->WriteNextLine(LOG_ERROR,CM_UNKNOWN_OPTION,ptr,CurrentCommand);return com;
					}
				} else {
					mesg->WriteNextLine(LOG_ERROR,CM_INVALID_OPTION,ptr,CurrentCommand);return com;
				}
			}
		}



		//alg generic
		if (com == COM_ALGCONST || com == COM_ALG1ST || com == COM_ALG2ND || com == COM_ALGSLOPE 
			|| com == COM_ALGSTRIPE || com == COM_ALGPEAKFINDER || com == COM_ALGRADIAL) {
				if (_stricmp(ptr,"-linear")==0 && (com == COM_ALGPEAKFINDER)) {
					linear=1;
				} else {
					ptr2 = strtok_int(ptr, '=',&saveptr2);
					if (ptr2!=NULL && strlen(ptr2)>0) {
						if (strncmp(ptr2,"-add",2)==0) {
							ptr2 = strtok_int(NULL, '=',&saveptr2);
							if (ptr2)
								sscanf_s(ptr2,"%f",&add);
							else {
								mesg->WriteNextLine(LOG_ERROR,CM_SYNTAX_ERROR,ptr,CurrentCommand);return com;
							}
						} else if (strncmp(ptr2,"-multiply",6)==0) {
							ptr2 = strtok_int(NULL, '=',&saveptr2);
							if (ptr2)
								sscanf_s(ptr2,"%f",&multiply);
							else {
								mesg->WriteNextLine(LOG_ERROR,CM_SYNTAX_ERROR,ptr,CurrentCommand);return com;
							}
						} else if (strncmp(ptr2,"-lowest",4)==0 && (com == COM_ALGSLOPE || com == COM_ALGSTRIPE 
							|| com == COM_ALGPEAKFINDER)) {
								ptr2 = strtok_int(NULL, '=',&saveptr2);
								if (ptr2)
									sscanf_s(ptr2,"%f",&Lowest);
								else {
									mesg->WriteNextLine(LOG_ERROR,CM_SYNTAX_ERROR,ptr,CurrentCommand);return com;
								}
						} else if (strncmp(ptr2,"-highest",5)==0 && (com == COM_ALGSLOPE || com == COM_ALGSTRIPE)) {
							ptr2 = strtok_int(NULL, '=',&saveptr2);
							if (ptr2)
								sscanf_s(ptr2,"%f",&Highest);
							else {
								mesg->WriteNextLine(LOG_ERROR,CM_SYNTAX_ERROR,ptr,CurrentCommand);return com;
							}
						} else if (strncmp(ptr2,"-near",4)==0 && (com == COM_ALGRADIAL)) {
							ptr2 = strtok_int(NULL, '=',&saveptr2);
							if (ptr2)
								sscanf_s(ptr2,"%f",&Lowest);
							else {
								mesg->WriteNextLine(LOG_ERROR,CM_SYNTAX_ERROR,ptr,CurrentCommand);return com;
							}
						} else if (strncmp(ptr2,"-far",5)==0 && (com == COM_ALGRADIAL)) {
							ptr2 = strtok_int(NULL, '=',&saveptr2);
							if (ptr2)
								sscanf_s(ptr2,"%f",&Highest);
							else {
								mesg->WriteNextLine(LOG_ERROR,CM_SYNTAX_ERROR,ptr,CurrentCommand);return com;
							}
						} else if (strcmp(ptr2,"-x")==0 && (com == COM_ALGRADIAL)) {
							ptr2 = strtok_int(NULL, '=',&saveptr2);
							if (ptr2)
								sscanf_s(ptr2,"%f",&xx1);
							else {
								mesg->WriteNextLine(LOG_ERROR,CM_SYNTAX_ERROR,ptr,CurrentCommand);return com;
							}
						} else if (strcmp(ptr2,"-y")==0 && (com == COM_ALGRADIAL)) {
							ptr2 = strtok_int(NULL, '=',&saveptr2);
							if (ptr2)
								sscanf_s(ptr2,"%f",&yy1);
							else {
								mesg->WriteNextLine(LOG_ERROR,CM_SYNTAX_ERROR,ptr,CurrentCommand);return com;
							}
						} else if (strncmp(ptr2,"-minval",4)==0 && (com == COM_ALGSLOPE || com == COM_ALGSTRIPE 
							|| com == COM_ALGPEAKFINDER || com == COM_ALGRADIAL)) {
								ptr2 = strtok_int(NULL, '=',&saveptr2);
								if (ptr2)
									sscanf_s(ptr2,"%f",&ValueAtLowest);
								else {
									mesg->WriteNextLine(LOG_ERROR,CM_SYNTAX_ERROR,ptr,CurrentCommand);return com;
								}
						} else if (strncmp(ptr2,"-maxval",4)==0 && (com == COM_ALGSLOPE || com == COM_ALGSTRIPE 
							|| com == COM_ALGPEAKFINDER || com == COM_ALGRADIAL)) {
								ptr2 = strtok_int(NULL, '=',&saveptr2);
								if (ptr2)
									sscanf_s(ptr2,"%f",&ValueAtHighest);
								else {
									mesg->WriteNextLine(LOG_ERROR,CM_SYNTAX_ERROR,ptr,CurrentCommand);return com;
								}
						} else if (strncmp(ptr2,"-radius",4)==0 && (com == COM_ALGPEAKFINDER)) {
							ptr2 = strtok_int(NULL, '=',&saveptr2);
							if (ptr2)
								sscanf_s(ptr2,"%f",&Radius);
							else {
								mesg->WriteNextLine(LOG_ERROR,CM_SYNTAX_ERROR,ptr,CurrentCommand);return com;
							}
						} else if (strncmp(ptr2,"-scanradius",4)==0 && (com == COM_ALGPEAKFINDER)) {
							ptr2 = strtok_int(NULL, '=',&saveptr2);
							if (ptr2)
								sscanf_s(ptr2,"%f",&Scanradius);
							else {
								mesg->WriteNextLine(LOG_ERROR,CM_SYNTAX_ERROR,ptr,CurrentCommand);return com;
							}
						} 
						else {
							mesg->WriteNextLine(LOG_ERROR,CM_UNKNOWN_OPTION,ptr,CurrentCommand);return com;
						}
					} else {
						mesg->WriteNextLine(LOG_ERROR,CM_INVALID_OPTION,ptr,CurrentCommand);return com;
					}
				}
		}

		ptr = strtok_int(NULL, ' ', &saveptr1);
	}

	//afterburner
	if (com == COM_FOCUSQUAD) {
		if (quadx<-1110 || quady<-1110) {
			mesg->WriteNextLine(LOG_ERROR,"%s: no quad specified (needs -x and -y)",COM_FOCUSQUAD_CMD);
			return -1;
		}
		x00 = quadx*32.f*4096.f;
		x11 = (quadx+1.f)*32.f*4096.f;
		y00 = quady*32.f*4096.f;
		y11 = (quady+1.f)*32.f*4096.f;
	}

	if (com == COM_WRITEQUAD) {
		if (quadx<-1110 || quady<-1110) {
			mesg->WriteNextLine(LOG_ERROR,"%s: no quad specified (needs -x and -y)",COM_WRITEQUAD_CMD);
			return -1;
		}
		x00 = quadx*32.f*4096.f;
		x11 = (quadx+1.f)*32.f*4096.f;
		y00 = quady*32.f*4096.f;
		y11 = (quady+1.f)*32.f*4096.f;
	}

	if (com == COM_SETGRID) {
		if (gridx<0 || gridy<0) {
			mesg->WriteNextLine(LOG_ERROR,"%s: no grid specified (needs -x and -y)",COM_SETGRID_CMD);
			return -1;
		}
	}

	if (com == COM_SETGRIDBORDER) {
		if (gridx<0 || gridy<0) {
			mesg->WriteNextLine(LOG_ERROR,"%s: no grid specified (needs -x and -y)",COM_SETGRIDBORDER_CMD);
			return -1;
		}
	}

	if (com == COM_BREAKATGRID) {
		if (gridx<0 || gridy<0 || max<0) {
			mesg->WriteNextLine(LOG_ERROR,"%s: no grid specified (needs -x and -y) or no -max",COM_BREAKATGRID_CMD);
			return -1;
		}
	}

	if (com == COM_SETSINGLEPOINT) {
		if (gridx<-999999 || gridy<-999999) {
			mesg->WriteNextLine(LOG_ERROR,"%s: no coordinates specified (needs -x and -y)", CurrentCommand);
			return -1;
		}
	}

	if (com == COM_DIVIDEBETWEEN) {
		if (xx1 <-999999 || yy1<-999999 || xx2<-999999 || yy2<-999999) {
			mesg->WriteNextLine(LOG_ERROR,"%s: no coordinates specified (needs -x1, -x2, -y1 and -y2)",COM_DIVIDEBETWEEN_CMD);
			return -1;
		}
	}

	if (com == COM_CREATEPOLYGON) {
		if (xx1 <-999999 || yy1<-999999 || xx2<-999999 || yy2<-999999) {
			mesg->WriteNextLine(LOG_ERROR,"%s: no initial vertices specified (needs -x1, -x2, -y1 and -y2)", CurrentCommand);
			return -1;
		}
		if (!polygon_name) {
			mesg->WriteNextLine(LOG_ERROR,"%s: no name defined", CurrentCommand);
			return -1;
		}
	}

	if (com == COM_ADDVERTEXTOPOLYGON) {
		if (xx1 <-999999 || yy1<-999999 ) {
			mesg->WriteNextLine(LOG_ERROR,"%s: no vertex specified (needs -x and -x)", CurrentCommand);
			return -1;
		}
		if (!polygon_name) {
			mesg->WriteNextLine(LOG_ERROR,"%s: no name defined", CurrentCommand);
			return -1;
		}
	}

	if (com == COM_ACTIVATEVISIBLEVERTICES) {
		if (xx1 <-999999 || yy1<-999999 ) {
			mesg->WriteNextLine(LOG_ERROR,"%s: no view point specified (needs -x and -x)",COM_ACTIVATEVISIBLEVERTICES_CMD);
			return -1;
		}
	}

	if (com == COM_STENCILPOLGON) {		
		if (!polygon_name) {
			mesg->WriteNextLine(LOG_ERROR,"%s: no name defined",COM_STENCILPOLGON_CMD);
			return -1;
		}
	}

	if (com == COM_BREAKLINE) {
		if (z<-111111 )	{
			mesg->WriteNextLine(LOG_ERROR,"%s: no height specified (needs -z)",COM_BREAKLINE_CMD);
			return -1;
		}
	}



	if (com == COM_SETPOINTS || com == COM_SETPOINTSPERQUAD || 
		com == COM_SETMAXPOINTS || com == COM_SETMAXPOINTSPERQUAD) {
			if (npoints<0) {
				mesg->WriteNextLine(LOG_ERROR,"%s: number of points not specified (needs -n)",CurrentCommand);
				return -1;
			}
	}

	if (com == COM_FILTER) {
		if (npoints<0) {
			mesg->WriteNextLine(LOG_ERROR,"%s: number of points not specified (needs -n)",CurrentCommand);
			return -1;
		}
	}

	if (com == COM_READBMP) {
		if (npoints<0) 
		npoints=1;
	}

	if (com == COM_ALGCONST || com == COM_ALG1ST|| com == COM_ALG2ND || com == COM_ALGSLOPE 
		|| com == COM_ALGSTRIPE || com == COM_ALGPEAKFINDER) {
			if (add<0 && multiply<0) {
				mesg->WriteNextLine(LOG_WARNING,"%s: no add or multiply factor specified (assuming -multiply=1)",cmd);
				multiply=1.;
			}
	}

	if (add<0) add=0.;
	if (multiply<0) multiply=0.;

	if (com == COM_GUIREQUESTVERSION) {
		if (!myflagname) {
			mesg->WriteNextLine(LOG_ERROR,"%s: missing -value", CurrentCommand);
			myflagname="0.0";
		} 
	}

	if (com == COM_SETFLAG) {
		if (!myflagname) {
			mesg->WriteNextLine(LOG_ERROR,"%s: missing -name",COM_SETFLAG_CMD);
		} else {
			if (myflagvalue) {
				utils->SetValue(myflagname,myflagvalue);
				if (hidden) utils->SetHidden(myflagname);
			} else {
				utils->AddFlag(myflagname);
				if (hidden) utils->SetHidden(myflagname);
			} 
			if (unselect) utils->DisableFlag(myflagname);
		}
	}

#if 1
	if (com == COM_LOGFILE) {
		if (!myflagname) {
			mesg->WriteNextLine(LOG_ERROR,"%s: missing -value", CurrentCommand);
		} else {
			if (fopen_s(&logfile,myflagname,"w")) {
				mesg->WriteNextLine(LOG_ERROR,"%s: unable to open %s", CurrentCommand,myflagname);
			} else {
				mesg->SetLogFile(logfile);
				time_t rawtime;
				time ( &rawtime );
				mesg->WriteNextLine(LOG_INFO,"%s: Start logging to %s on %s", CurrentCommand,myflagname,ctime (&rawtime));
			}
		}
	}
#endif

    if (com == COM_ADDGAME) {
		if (npoints<0) {
			mesg->WriteNextLine(LOG_ERROR,"%s: game number (-n) not set", CurrentCommand);
		} else {
			mesg->WriteNextLine(LOG_INFO,"%s: Game %s added", CurrentCommand, myflagname);
			game[npoints]=myflagname;
		}
	}

    if (com == COM_SETGAMEPLUGINFILE) {
		if (npoints<0 && !gamemode) {
			mesg->WriteNextLine(LOG_ERROR,"%s: game number (-n) not set",COM_SETGAMEPLUGINFILE_CMD);
		} else if (npoints<0 && gamemode) {
			plugin[gamemode]=myflagname;
		} else {
			plugin[npoints]=myflagname;
		}
	}

	if (com == COM_SETGAMESEARCHPATTERN) {
		if (npoints<0 && !gamemode) {
			mesg->WriteNextLine(LOG_ERROR,"%s: game number (-n) not set",COM_SETGAMESEARCHPATTERN_CMD);
		} else if (npoints<0 && gamemode) {
			pattern[gamemode]=myflagname;
		} else {
			pattern[npoints]=myflagname;
		}
	}

	if (com == COM_SETGAMESTDWS) {
		if (npoints<0 && !gamemode) {
			mesg->WriteNextLine(LOG_ERROR,"%s: game number (-n) not set",COM_SETGAMESTDWS_CMD);
		} else if (npoints<0 && gamemode) {
			std_ws[gamemode]=myflagname;
		} else {
			std_ws[npoints]=myflagname;
		}
	}

	if (com == COM_GAMEMODE) {
		if (myflagname) {
			gamemode = 0;
			for (int i=0;i<MAX_GAMES;i++) {
				if (game[i] && _stricmp(game[i],myflagname) == 0) {
					mesg->WriteNextLine(LOG_INFO,"%s: game set to %s (-n=%i)",COM_GAMEMODE_CMD,myflagname,i);
					gamemode = i;
					utils->SetValue("_gamemode",game[gamemode]);
					utils->SetHidden("_gamemode");
				}
			}
			if (!gamemode)
				mesg->WriteNextLine(LOG_ERROR,"%s: game %s not defined",COM_GAMEMODE_CMD,myflagname);
		}
		else
			mesg->WriteNextLine(LOG_ERROR,"%s: game not defined (needs -name)",COM_GAMEMODE_CMD,myflagname);

	}

	return com;

}

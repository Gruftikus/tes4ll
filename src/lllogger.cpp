

//#include "StdAfx.h"

#include "..\include\lllogger.h"
//#include <string.h>
#include <stdio.h>
#include <iostream>


llLogger::llLogger() {
	write_pointer =  MH_NUM_LINES;
	read_pointer = MH_NUM_LINES - 1;
	tot_lines = counter = 0;
	for (int i=0;i<MH_NUM_LINES;i++) {
		lines[i]= NULL;
	}
	logfile = NULL;
}

char *llLogger::ReadNextLine(void) {

	if (tot_lines == 0) return NULL;

	if (lines[read_pointer]) {
		delete lines[read_pointer];
		lines[read_pointer] = NULL;
	} 

	read_pointer++;
	if (read_pointer==MH_NUM_LINES)  read_pointer = 0;
	tot_lines--;

	char *s = ":";
	if (lines[read_pointer]) {
		s=lines[read_pointer];
	} 
	
	return s;
}

int llLogger::WriteNextLine(int level, char *format, ...) {

	if (level<1) return 1;
	if (tot_lines == MH_NUM_LINES-1) return 0;
	
	if (write_pointer == MH_NUM_LINES) write_pointer = 0;
	counter++;
	tot_lines++;
	
	va_list args;
	va_start(args,format);
	char tmp[MH_MAX_LENGTH];
	char tmp2[MH_MAX_LENGTH];

	vsprintf_s(tmp,MH_MAX_LENGTH,format,args);
	//vsprintf(tmp,format,args);
	va_end(args);
	
	if (level == MH_DEBUG)
		sprintf_s(tmp2,MH_MAX_LENGTH,"%i [Debug] %s",counter,tmp); 
	else if (level == MH_INFO)
		sprintf_s(tmp2,MH_MAX_LENGTH,"%i [Info] %s",counter,tmp); 
	else if (level == MH_WARNING)
		sprintf_s(tmp2,MH_MAX_LENGTH,"%i [Warning] %s",counter,tmp); 
	else if (level == MH_ERROR)
		sprintf_s(tmp2,MH_MAX_LENGTH,"%i [Error] %s",counter,tmp); 
	else if (level == MH_FATAL)
		sprintf_s(tmp2,MH_MAX_LENGTH,"%i [Fatal] %s",counter,tmp); 
	else if (level == MH_ECHO)
		sprintf_s(tmp2,MH_MAX_LENGTH,"%i [Echo] %s",counter,tmp); 
    else if (level == MH_COMMAND)
		sprintf_s(tmp2,MH_MAX_LENGTH,"%i [Command] %s",counter,tmp);
	else if (level == MH_ALGORITHM)
		sprintf_s(tmp2,MH_MAX_LENGTH,"%i [Algorithm] %s",counter,tmp);
	else sprintf_s(tmp2,MH_MAX_LENGTH,"%i: %s",counter,tmp); 
	
#if 1
	if (logfile) {
		if (level == MH_DEBUG)
			fprintf(logfile,"%i [Debug] %s\n",counter,tmp); 
		else if (level == MH_INFO)
			fprintf(logfile,"%i [Info] %s\n",counter,tmp); 
		else if (level == MH_WARNING)
			fprintf(logfile,"%i [Warning] %s\n",counter,tmp); 
		else if (level == MH_ERROR)
			fprintf(logfile,"%i [Error] %s\n",counter,tmp); 
		else if (level == MH_FATAL)
			fprintf(logfile,"%i [Fatal] %s\n",counter,tmp); 
		else if (level == MH_ECHO)
			fprintf(logfile,"%i [Echo] %s\n",counter,tmp); 
		else if (level == MH_COMMAND)
			fprintf(logfile,"%i [Command] %s\n",counter,tmp); 
		else if (level == MH_ALGORITHM)
			fprintf(logfile,"%i [Algorithm] %s\n",counter,tmp); 
		else fprintf(logfile,"%i: %s\n",counter,tmp); 
	}
#endif

	lines[write_pointer] = new char[strlen(tmp2)+1];
	strcpy_s(lines[write_pointer],strlen(tmp2)+1,tmp2);
	write_pointer++;
	
	return 1;
}




//#include "StdAfx.h"

#include "..\include\lllogger.h"
//#include <string.h>
#include <stdio.h>
#include <iostream>


llLogger::llLogger() {
	write_pointer =  MH_NUM_LINES;
	read_pointer = tot_lines = counter = 0;
	for (int i=0;i<MH_NUM_LINES;i++) {
		lines[i]= new char[MH_MAX_LENGTH];
		length[i]=0;
	}
	logfile = NULL;
}

char *llLogger::ReadNextLine(void) {
	if (tot_lines == 0) return NULL;
	char *s = ":";
	if (length[read_pointer]) {
		s=lines[read_pointer];
	} 
	length[read_pointer] =0;
	read_pointer++;
	if (read_pointer==MH_NUM_LINES)  read_pointer=0;
	tot_lines--;
	return s;
}

int llLogger::WriteNextLine(int level, char *format, ...) {

	if (write_pointer == MH_NUM_LINES) write_pointer=0;
	counter++;

	if (level<1) return 1;
	if (tot_lines == MH_NUM_LINES-1) return 0;
	va_list args;
	va_start(args,format);
	char tmp[MH_MAX_LENGTH];
	vsprintf_s(tmp,MH_MAX_LENGTH-50,format,args);
	
	//vsprintf(tmp,format,args);
	va_end(args);
	
	if (level == MH_DEBUG)
		sprintf_s(lines[write_pointer],500,"%i [Debug] %s",counter,tmp); 
	else if (level == MH_INFO)
		sprintf_s(lines[write_pointer],500,"%i [Info] %s",counter,tmp); 
	else if (level == MH_WARNING)
		sprintf_s(lines[write_pointer],500,"%i [Warning] %s",counter,tmp); 
	else if (level == MH_ERROR)
		sprintf_s(lines[write_pointer],500,"%i [Error] %s",counter,tmp); 
	else if (level == MH_FATAL)
		sprintf_s(lines[write_pointer],500,"%i [Fatal] %s",counter,tmp); 
	else if (level == MH_ECHO)
		sprintf_s(lines[write_pointer],500,"%i [Echo] %s",counter,tmp); 
    else if (level == MH_COMMAND)
		sprintf_s(lines[write_pointer],500,"%i [Command] %s",counter,tmp);
	else if (level == MH_ALGORITHM)
		sprintf_s(lines[write_pointer],500,"%i [Algorithm] %s",counter,tmp);
	else sprintf_s(lines[write_pointer],500,"%i: %s",counter,tmp); 
	
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

	length[write_pointer] = strlen(tmp);
	write_pointer++;
	tot_lines++;
	return 1;
}


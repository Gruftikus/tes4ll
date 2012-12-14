#ifndef _PLLMH_H_
#define _PLLMH_H_

#include <iostream>
#include <stdarg.h>

#define MH_NUM_LINES 500
#define MH_MAX_LENGTH 50000

#define MH_DEBUG	0
#define MH_INFO		1
#define MH_WARNING	2
#define MH_ERROR	3
#define MH_FATAL	5
#define MH_ECHO  	4
#define MH_COMMAND  	6
#define MH_ALGORITHM  	7


class llLogger {

	FILE *logfile;

public:

	llLogger();
	char *ReadNextLine(void);
	int   WriteNextLine(int level, char *format, ...);

	unsigned int write_pointer, read_pointer, tot_lines;
	char *lines[MH_NUM_LINES];
	int counter;

	void SetLogFile(FILE *myfile) {
		logfile=myfile;
	};

	void Log(const char * log) {
		if (logfile) fprintf(logfile,"%s",log);
	};

	void AddToLine(char * add) {
		sprintf_s(lines[write_pointer-1],MH_MAX_LENGTH,"%s%s",lines[write_pointer-1],add);
	}
	void AddToLine(char add) {
		sprintf_s(lines[write_pointer-1],MH_MAX_LENGTH,"%s%c",lines[write_pointer-1],add);
	}
	void AddToLine(char * add, float f) {
		sprintf_s(lines[write_pointer-1],MH_MAX_LENGTH,"%s%s%f",lines[write_pointer-1],add,f);
	}

	void Dump(void) {		
		char *x = ReadNextLine();
		while (x) {
			std::cout << x << std::endl;
			x = ReadNextLine();
		}			
	}


};


#endif

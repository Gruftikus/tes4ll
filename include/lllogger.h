#ifndef _PLLLOGGER_H_
#define _PLLLOGGER_H_

#include <iostream>
#include <stdarg.h>

#define LOG_NUM_LINES  500
#define LOG_MAX_LENGTH 50000

#define LOG_DEBUG	0
#define LOG_INFO		1
#define LOG_WARNING	2
#define LOG_ERROR	3
#define LOG_FATAL	5
#define LOG_ECHO  	4
#define LOG_COMMAND  	6
#define LOG_ALGORITHM  	7

class      llLogger;
llLogger* _llLogger();
llLogger& _fllLogger();

class llLogger {

private:

	FILE *logfile;

	unsigned int write_pointer, read_pointer, tot_lines;
	char *lines[LOG_NUM_LINES];
	int counter;

public:

	llLogger();
	char *ReadNextLine(void);
	int   WriteNextLine(int _level, char *_format, ...);

	void SetLogFile(FILE *_file) {
		logfile = _file;
	};

	void Log(const char *_log) {
		if (logfile) fprintf(logfile,"%s", _log);
	};

	void AddToLine(char *_add) {
		char *delme = lines[write_pointer-1];
		unsigned int len = strlen(delme) + 1 + strlen(_add);
		lines[write_pointer-1] = new char[len];
		sprintf_s(lines[write_pointer-1], len, "%s%s", delme, _add);
		delete delme;
	}
	void AddToLine(char _add) {
		char *delme = lines[write_pointer-1];
		unsigned int len = strlen(delme) + 2;
		lines[write_pointer-1] = new char[len];
		sprintf_s(lines[write_pointer-1], len, "%s%c", delme, _add);
		delete delme;
	}
	void AddToLine(char * _add, double _f, int _width = 1) {
		char *delme = lines[write_pointer-1];
		unsigned int len = strlen(delme) + 100 + strlen(_add);
		lines[write_pointer-1] = new char[len];
		sprintf_s(lines[write_pointer-1], len, "%s%s%*f", delme, _add, _width, _f);
		delete delme;
	}
	void AddToLine(char * _add, float _f, int _width = 1) {
		char *delme = lines[write_pointer-1];
		unsigned int len = strlen(delme) + 100 + strlen(_add);
		lines[write_pointer-1] = new char[len];
		sprintf_s(lines[write_pointer-1], len, "%s%s%*f", delme, _add, _width, _f);
		delete delme;
	}
	void AddToLine(char * _add, int _f) {
		char *delme = lines[write_pointer-1];
		unsigned int len = strlen(delme) + 100 + strlen(_add);
		lines[write_pointer-1] = new char[len];
		sprintf_s(lines[write_pointer-1], len, "%s%s%i", delme, _add, _f);
		delete delme;
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

#ifndef _PLLUTILS_H_
#define _PLLUTILS_H_

#define MAX_LINES				10000
#define MAX_FLAGS				2000
#define MAX_GAMES				10

#include <iostream>
#include <stdarg.h>

#include "../include/lllogger.h"

char *strtok_int(char *_ptr, const char _delim, char **_saveptr1);

class     llUtils;
llUtils* _llUtils();
llUtils& _fllUtils();

class llUtils {

 private:

	llLogger *mesg;
	
	//Flag handling:
	char       *flag_list[MAX_FLAGS];
	const char *flag_value[MAX_FLAGS];
	char       *flag_description[MAX_FLAGS]; //for dropdowns
	int         flag_enable[MAX_FLAGS];
	int         flag_hidden[MAX_FLAGS];
	unsigned int num_flags;

	char *crunch_string, *crunch_saveptr, *crunch_current;

 public:

    //constructor
	llUtils();

	int   CrunchStart(char *_s);
	int   CrunchNext(void);
	char *CrunchCurrent(void) {return crunch_current;};

	//tool functions used everywhere:
	void StripQuot(char **_tmp);
	void StripSpaces(char **_tmp);

	int AddFlag(const char *_name);

	int EnableFlag(const char *_name);
	int DisableFlag(const char *_name);
	int IsEnabled(const char *_name);

	int SetValue(const char *_name, const char *_value);
	const char* GetValue(const char *_name);
	
	int SetHidden(const char *_name);

	int   SetDescription(const char *_name, char *_value);
	char *GetDescription(const char *_name);
	char *GetFlagViaDescription(const char *_value);

	int   WriteFlags(FILE *wfile);

	int MyIsPrint(char _c) {
		if (_c == '\n') return 1;
		if (_c == '\r') return 1;
		if (_c == '\t') return 1;

		if (_c>= 0x20 && _c <=0x7E) return 1;
		return 0;
	}

	//********************
	//Global variables
	//********************

	float x00, y00, x11, y11; //focus


};

#endif

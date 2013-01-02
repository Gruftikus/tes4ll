

//#include "StdAfx.h"

#include "..\include\llutils.h"
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


char *strtok_old(char *string, const char *seps, char **context) {
	//The "original" strtok
	char *head;  /* start of word */
	char *tail;  /* end of word */

	/* If we're starting up, initialize context */
	if (string) {
		*context = string;
	}
	/* Get potential start of this next word */
	head = *context;
	if (head == NULL) {
		return NULL;
	}

	/* Skip any leading separators */
	while (*head && strchr(seps, *head)) {
		head++;
	}

	/* Did we hit the end? */
	if (*head == 0) {
		/* Nothing left */
		*context = NULL;
		return NULL;
	}

	/* skip over word */
	tail = head;
	while (*tail && !strchr(seps, *tail)) {
		tail++;
	}

	/* Save head for next time in context */
	if (*tail == 0) {
		*context = NULL;
	}
	else {
		*tail = 0;
		tail++;
		*context = tail;
	}

	/* Return current word */
	return head;

}

char * strtok_int(char *ptr, const char delim, char **saveptr1) {
	//Wrapper
	int foundquot=0;
	char *ptr1;
	char delim2[2];
	sprintf_s(delim2,2,"%c",delim);
	if (*saveptr1) {
		for (unsigned int h=0;h<strlen(*saveptr1);h++) {
			if ((*saveptr1)[h]=='\"' && foundquot) {foundquot=0;if (h>0 && (*saveptr1)[h-1]=='\\') foundquot=1;}
			else if ((*saveptr1)[h]=='\"' && !foundquot) {foundquot=1;if (h>0 && (*saveptr1)[h-1]=='\\') foundquot=0;}
			if ((*saveptr1)[h]==delim && !foundquot) (*saveptr1)[h]='§';
		}
	} 
	foundquot=0;
	if (ptr) {
		for (unsigned int h=0;h<strlen(ptr);h++) {
			if ((ptr)[h]=='\"' && foundquot) {foundquot=0;if (h>0 && (ptr)[h-1]=='\\') foundquot=1;}
			else if ((ptr)[h]=='\"' && !foundquot) {foundquot=1;if (h>0 && (ptr)[h-1]=='\\') foundquot=0;}
			if ((ptr)[h]==delim && !foundquot) (ptr)[h]='§';
		}
	} 
	ptr1 = strtok_old(ptr, "§", saveptr1);
	foundquot=0;
	if (*saveptr1) {
		for (unsigned int h=0;h<strlen(*saveptr1);h++) {
			if ((*saveptr1)[h]=='\"' && foundquot) {foundquot=0;if (h>0 && (*saveptr1)[h-1]=='\\') foundquot=1;}
			else if ((*saveptr1)[h]=='\"' && !foundquot) {foundquot=1;if (h>0 && (*saveptr1)[h-1]=='\\') foundquot=0;}
			if ((*saveptr1)[h]=='§' && !foundquot) (*saveptr1)[h]=delim;
		}
	} 
	foundquot=0;
	if (ptr1) {
		for (unsigned int h=0;h<strlen(ptr1);h++) {
			if ((ptr1)[h]=='\"' && foundquot) {
				foundquot=0;
				if (h>0 && (ptr1)[h-1]=='\\') {
					foundquot=1;
					//for (unsigned int j=h-1;j<strlen(ptr1)-1;j++) (ptr1)[j]=(ptr1)[j+1];
					//(ptr1)[strlen(ptr1)-1]='\0';
				}
			}
			else if ((ptr1)[h]=='\"' && !foundquot) {
				foundquot=1;
				if (h>0 && (ptr1)[h-1]=='\\') {
					foundquot=0;
					//for (unsigned int j=h-1;j<strlen(ptr1)-1;j++) (ptr1)[j]=(ptr1)[j+1];
					//(ptr1)[strlen(ptr1)-1]='\0';
				}
			}
			if ((ptr1)[h]=='§' && !foundquot) (ptr1)[h]=delim;
		}
	} 
	return ptr1;
}



int ReadUShort(FILE *fptr,short unsigned *n,int swap)
{
   unsigned char *cptr,tmp;

   if (fread(n,2,1,fptr) != 1)
       return(0);
   if (swap) {
       cptr = (unsigned char *)n;
       tmp = cptr[0];
       cptr[0] = cptr[1];
       cptr[1] =tmp;
   }
   return(1);
}

/*
   Write a possibly byte swapped unsigned short integer
*/
int WriteUShort(FILE *fptr,short unsigned n,int swap)
{
   unsigned char *cptr,tmp;

	if (!swap) {
   	if (fwrite(&n,2,1,fptr) != 1)
      	return(0);
   } else {
      cptr = (unsigned char *)(&n);
      tmp = cptr[0];
      cptr[0] = cptr[1];
      cptr[1] =tmp;
      if (fwrite(&n,2,1,fptr) != 1)
         return(0);
   }
   return(1);
}


int ReadUInt(FILE *fptr,unsigned int *n,int swap)
{
   unsigned char *cptr,tmp;

   if (fread(n,4,1,fptr) != 1)
       return(0);
   if (swap) {
       cptr = (unsigned char *)n;
       tmp = cptr[0];
       cptr[0] = cptr[3];
       cptr[3] = tmp;
       tmp = cptr[1];
       cptr[1] = cptr[2];
       cptr[2] = tmp;
   }
   return(1);
}

int ReadInt(FILE *fptr, int *n,int swap)
{
   unsigned char *cptr,tmp;

   if (fread(n,4,1,fptr) != 1)
       return(0);
   if (swap) {
       cptr = (unsigned char *)n;
       tmp = cptr[0];
       cptr[0] = cptr[3];
       cptr[3] = tmp;
       tmp = cptr[1];
       cptr[1] = cptr[2];
       cptr[2] = tmp;
   }

   return(1);
}

int WriteInt(FILE *fptr,int n,int swap)
{
   unsigned char *cptr,tmp;

	if (!swap) {
   	if (fwrite(&n,4,1,fptr) != 1)
      	return(0);
   } else {
      cptr = (unsigned char *)(&n);
      tmp = cptr[0];
      cptr[0] = cptr[3];
      cptr[3] = tmp;
      tmp = cptr[1];
      cptr[1] = cptr[2];
      cptr[2] = tmp;
      if (fwrite(&n,4,1,fptr) != 1)
         return(0);
   }
   return(1);
}

/*
   Write a possibly byte swapped unsigned integer
*/
int WriteUInt(FILE *fptr,unsigned int n,int swap)
{
   unsigned char *cptr,tmp;

   if (!swap) {
      if (fwrite(&n,4,1,fptr) != 1)
         return(0);
   } else {
      cptr = (unsigned char *)(&n);
      tmp = cptr[0];
      cptr[0] = cptr[3];
      cptr[3] = tmp;
      tmp = cptr[1];
      cptr[1] = cptr[2];
      cptr[2] = tmp;
      if (fwrite(&n,4,1,fptr) != 1)
         return(0);
   }
   return(1);
}


llUtils& _fllUtils()
{
    static llUtils* ans = new llUtils();
    return *ans;
}

llUtils * _llUtils()
{
    return &_fllUtils();
}

//constructor
llUtils::llUtils() {
	num_flags = 0;
	mod_list.resize(0);
}


void llUtils::StripQuot(char **_tmp) {
	if ((*_tmp)[0] == '\"') (*_tmp)++;
	if ((*_tmp)[strlen(*_tmp)-1] == '\"') (*_tmp)[strlen(*_tmp)-1]='\0';
	for (unsigned int h=0;h<strlen(*_tmp);h++) {
		if ((*_tmp)[h]=='\"') {
			if (h>0 && (*_tmp)[h-1]=='\\') {
				for (unsigned int j=h-1;j<strlen(*_tmp)-1;j++) (*_tmp)[j]=(*_tmp)[j+1];
				(*_tmp)[strlen(*_tmp)-1]='\0';
			}
		}
	}
}


void llUtils::StripSpaces(char **_partc) {
	while (**_partc==' ') (*_partc)++;
	if (strlen(*_partc)) {
		int partend=strlen(*_partc)-1;

		while ((*_partc)[partend]==' ' && partend>=0) {
			(*_partc)[partend]='\0';
			partend--;
		}
	}
}


int llUtils::CrunchStart(char *_s) {
	crunch_string = new char[strlen(_s)+1];
	strcpy_s(crunch_string,strlen(_s)+1,_s);
	crunch_current = NULL;
	crunch_saveptr = NULL;
	return 1;
}

int llUtils::CrunchNext(void) {
	if (!crunch_current) {
		crunch_current = strtok_int(crunch_string, ',', &crunch_saveptr);
		return 1;
	}
	crunch_current = strtok_int(NULL, ',', &crunch_saveptr);
	if (crunch_current) return 1;

	return 0;
}

int llUtils::AddFlag(const char *_name) {
	if (EnableFlag(_name)) return 0; //already there
	char * tmp = new char[strlen(_name)+2];
	strcpy_s(tmp,strlen(_name)+1,_name);
	if (num_flags == MAX_FLAGS) {
		mesg->WriteNextLine(LOG_ERROR,"Maximal number of flags (%i) reached, flag %s not added", MAX_FLAGS, _name);
		return 0;
	}
	char *val = NULL;
	for (unsigned int i = 0; i < strlen(tmp); i++) {
		if (tmp[i] == '=') {
			tmp[i] = '\0';
			val = tmp + i + 1;
			break;
		}
	}
	flag_list[num_flags] = tmp;
	flag_value[num_flags] = val;
	flag_description[num_flags] = NULL;
	flag_enable[num_flags] = 1;
	flag_hidden[num_flags] = 0;
	num_flags++;
	return 1;
}

int llUtils::EnableFlag(const char *_name) {
	//mesg->WriteNextLine(LOG_ERROR,"Enable flag [%s]",myname);
	for (unsigned int i=0;i<num_flags;i++) {
		if (_stricmp(_name,flag_list[i])==0) {
			flag_enable[i] = 1;
			return 1;
		}
	}
	return 0;
}

int llUtils::DisableFlag(const char *_name) {
	for (unsigned int i=0;i<num_flags;i++) {
		if (_stricmp(_name,flag_list[i])==0) {
			flag_enable[i] = 0;
			return 1;
		}
	}
	return 0;
}

int llUtils::IsEnabled(const char *_name) {
	unsigned int pos=strlen(_name);
	for (unsigned int i=0;i<strlen(_name);i++) {
		if (_name[i]=='=') pos=i;
	}
	for (unsigned int i=0;i<num_flags;i++) {
		if (_strnicmp(_name,flag_list[i],pos)==0 && pos==strlen(flag_list[i])) {
			if (pos==strlen(_name)) {
				return flag_enable[i];
			} else {
				char *val = (char *)GetValue(flag_list[i]);
				char * newname = new char[strlen(_name+pos+1)+1];
				strcpy_s(newname,strlen(_name+pos+1)+1,_name+pos+1);
				StripQuot(&newname);
				if (_stricmp(newname,val)==0) {					
					return flag_enable[i];
				}
			}
		}
	}
	return 0;
}

int llUtils::SetValue(const char *_name, const char *_value) {
	AddFlag(_name);
	for (unsigned int i=0;i<num_flags;i++) {
		if (_stricmp(_name,flag_list[i])==0) {
			flag_value[i] = _value;
			return 1;
		}
	}
	return 0;
}

int llUtils::SetHidden(const char *_name) {
	AddFlag(_name);
	for (unsigned int i=0;i<num_flags;i++) {
		if (_stricmp(_name,flag_list[i])==0) {
			flag_hidden[i] = 1;
			return 1;
		}
	}
	return 0;
}

const char* llUtils::GetValue(const char *_name) {
	int p=0;
	if (_stricmp(_name,"_flaglist")==0) {
		char tmp[LOG_MAX_LENGTH];
		sprintf_s(tmp,100000,"\0");
		for (unsigned int i=0;i<num_flags;i++) {
			if (flag_enable[i] && (_stricmp(flag_list[i],"_modlist")!=0) && !flag_hidden[i]) {
				int g=strlen(tmp);
				if (flag_value[i]) {
					if (p>0) sprintf_s(tmp,100000-g,"%s,%s=%s",tmp,flag_list[i],flag_value[i]); 
					else sprintf_s(tmp,100000,"%s=%s",flag_list[i],flag_value[i]); 
					p++;
				} else {
					if (p>0) sprintf_s(tmp,100000-g,"%s,%s",tmp,flag_list[i]); 
					else sprintf_s(tmp,100000,"%s",flag_list[i]); 
					p++;
				}
			}
		}
		char *tmp2 = new char[strlen(tmp)+1];
		strcpy_s(tmp2,strlen(tmp)+1,tmp);
		return tmp2;
	}
	for (unsigned int i=0;i<num_flags;i++) {
		if (_stricmp(_name,flag_list[i])==0) {
			if (flag_value[i]) return flag_value[i];
			if (flag_enable[i]) return "1";
			return "0";
		}
	}
	return NULL;
}

int llUtils::SetDescription(const char *_name, char *_value) {
	AddFlag(_name);
	for (unsigned int i=0;i<num_flags;i++) {
		if (_stricmp(_name,flag_list[i])==0) {
			flag_description[i] = _value;
			return 1;
		}
	}
	return 0;
}

char* llUtils::GetDescription(const char *_name) {
	for (unsigned int i=0;i<num_flags;i++) {
		if (_stricmp(_name,flag_list[i])==0) {
			return flag_description[i];
		}
	}
	return NULL;
}

char * llUtils::GetFlagViaDescription(const char *_value) {
	for (unsigned int i=0;i<num_flags;i++) {
		if (flag_description[i]) {
			if (_stricmp(_value,flag_description[i])==0) {
				return flag_list[i];
			}
		}
	}
	return NULL;
}


int llUtils::WriteFlags(FILE *wfile) {

	for (unsigned int i=0;i<num_flags;i++) {
		if (flag_enable[i] && !flag_hidden[i]) {
			if (flag_value[i]) {
				fprintf(wfile,"SetFlag -name=%s -value=\"%s\"\n",flag_list[i],flag_value[i]);
			} else {
				fprintf(wfile,"SetFlag -name=%s\n",flag_list[i]);
			} 
		} else if (!flag_hidden[i]) {
			if (flag_value[i]) {
				fprintf(wfile,"SetFlag -name=%s -value=\"%s\" -unselect\n",flag_list[i],flag_value[i]);
			} else {
				fprintf(wfile,"SetFlag -name=%s -unselect\n",flag_list[i]);
			} 
		} 
	}

	return 1;
}

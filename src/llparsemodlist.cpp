//base class for all algorithms

#include "..\include\llparsemodlist.h"

#include <string.h>
#include <stdio.h>
#include <windows.h>
#include <direct.h>
#include <winreg.h>

//constructor
llParseModList::llParseModList() : llWorker() {
	num_esp = 0;
	num_esp_sorted = 0;
	SetCommandName("ParseModList");
}

int llParseModList::RegisterOptions(void) {
	if (!llWorker::RegisterOptions()) return 0;

	return 1;
}

int llParseModList::Exec(void) {
	llWorker::Exec();

	HKEY keyHandle;
	char rgValue [1024];
	//    char fnlRes [1024];
	DWORD size1;
	DWORD Type;

	//seek for game dir, if not yet set by user
	if (!_llUtils()->IsEnabled("_gamedir")) {
		if( RegOpenKeyEx(    HKEY_LOCAL_MACHINE, 
			"SOFTWARE\\Bethesda Softworks\\Oblivion",0, 
			KEY_QUERY_VALUE, &keyHandle) == ERROR_SUCCESS) {
				size1 = 1023;
				RegQueryValueEx( keyHandle, "Installed Path", NULL, &Type, 
					(LPBYTE)rgValue,&size1);
				char *oblivion_path = new char[strlen(rgValue)+2];
				strcpy_s(oblivion_path, strlen(rgValue)+1, rgValue);
				_llLogger()->WriteNextLine(-LOG_INFO, "Game path is: %s", oblivion_path);
				_llUtils()->SetValue("_gamedir", oblivion_path);
		} else {
			_llLogger()->WriteNextLine(LOG_WARNING, "Game not installed, I will use the working directory.");
			_llUtils()->SetValue("_gamedir", ".");
			//DumpExit();
		}
		RegCloseKey(keyHandle);
	} else {
		_llLogger()->WriteNextLine(LOG_INFO,"Game path is: %s", _llUtils()->GetValue("_gamedir"));
	}

	const char *option = _llUtils()->GetValue("_modlist");

	if (!option) { // mod list not provided my main program
		char oblivion_app_path[1024];
		//open registry
		
		if( RegOpenKeyEx(    HKEY_CURRENT_USER, 
			"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders",0, 
			KEY_QUERY_VALUE, &keyHandle) == ERROR_SUCCESS) {
				size1=1023;
				strcpy_s(rgValue,1024,"\0");
				RegQueryValueEx( keyHandle, "Local Appdata", NULL, &Type, 
					(LPBYTE)rgValue,&size1); //win7
				if (strlen(rgValue) == 0) {
					strcpy_s(rgValue,1024,"\0");
					RegQueryValueEx( keyHandle, "Appdata", NULL, &Type, 
						(LPBYTE)rgValue,&size1); //win XP
				}
				if (strlen(rgValue) == 0) {
					_llLogger()->WriteNextLine(-LOG_FATAL,"Could not get Appdata path!");
				}
				strcpy_s(oblivion_app_path,1024,rgValue);
				_llLogger()->WriteNextLine(LOG_INFO,"Appdata path is: %s",oblivion_app_path);
		} else {
			_llLogger()->WriteNextLine(-LOG_FATAL,"Could not get Appdata path!");
		}
		RegCloseKey(keyHandle);

		char listname[2000];
		sprintf_s(listname,2000,"%s\\Oblivion\\plugins.txt\0", oblivion_app_path);

		FILE *fesplist = NULL;    
		if (fopen_s(&fesplist,listname,"r")) {
			_llLogger()->WriteNextLine(-LOG_FATAL,"Unable to open plugin file \"%s\"\n",listname);
		}

		esp_list[0] = new char[1024];
		while (fgets(esp_list[num_esp],1024,fesplist)) {
			if (esp_list[num_esp][0] != '#' && strlen(esp_list[num_esp])>5) {
				//remove the trailing \n
				if (num_esp == 256) {
					_llLogger()->WriteNextLine(-LOG_FATAL,"Too many plugins\n");
				}
				esp_list[num_esp][strlen(esp_list[num_esp])-1] = '\0';
				//cout << esp_list[num_esp];
				if (num_esp < 256) num_esp++;
				esp_list[num_esp] = new char[1024];
			}
		}
		fclose(fesplist);

		_llLogger()->WriteNextLine(LOG_INFO,"%i plugins will be used",num_esp);

		for (int i=0; i<num_esp; i++) {
			//open the esp
			WIN32_FILE_ATTRIBUTE_DATA fAt = {0};
			char tmpName2[2000];
			sprintf_s(tmpName2,2000, "%s", esp_list[i]); 
			wchar_t tmpName[2000]; 
			swprintf(tmpName, 2000, L"%s", tmpName2); 
			if (!GetFileAttributesEx(tmpName2,GetFileExInfoStandard,&fAt)) {
				_llLogger()->WriteNextLine(-LOG_FATAL, "The esp '%s' was not found", esp_list[i]);
				//cout << GetLastError() << endl;
			}
			FILETIME time = fAt.ftLastWriteTime;

			esp_list_sorted[num_esp_sorted]  = esp_list[i];
			time_list_sorted[num_esp_sorted] = time;
			num_esp_sorted++;

			for (int j=num_esp_sorted-1; j>0; j--) {  //quicksort
				if (CompareFileTime(&time_list_sorted[j-1], &time_list_sorted[j]) > 0) {
					FILETIME ttmp         = time_list_sorted[j-1];
					char * tmp            = esp_list_sorted[j-1];
					time_list_sorted[j-1] = time_list_sorted[j];
					esp_list_sorted[j-1]  = esp_list_sorted[j];
					time_list_sorted[j]   = ttmp;
					esp_list_sorted[j]    = tmp;
				}
			}
		}

		for (int j=0; j<num_esp_sorted; j++) {
			_llUtils()->AddMod(esp_list_sorted[j]);
			char * my_flag_list = new char[strlen(esp_list_sorted[j]) + 1];
			strcpy_s(my_flag_list, strlen(esp_list_sorted[j])+1, esp_list_sorted[j]);
			for (unsigned int jj=0; jj<strlen(my_flag_list); jj++) {
				if (*(my_flag_list+jj) == ' ') *(my_flag_list+jj)='_';
			}
			_llLogger()->WriteNextLine(LOG_INFO,"Mod flag: %s",my_flag_list);
			_llUtils()->AddFlag(my_flag_list);
		}

	} else { //list mod option provided
		char *ptr;          
		char *saveptr1 = NULL;
		char *list_string = new char[strlen(option)+1];
		strcpy_s(list_string, strlen(option)+1, option);
		ptr = strtok_int(list_string, ',', &saveptr1);
		while(ptr != NULL) {
			char *flag_list = new char[strlen(ptr)+1];
			strcpy_s(flag_list, strlen(ptr)+1, ptr);
			char *mod_list = new char[strlen(ptr)+1];
			strcpy_s(mod_list, strlen(ptr)+1, ptr);
			_llUtils()->AddMod(mod_list);
			for (unsigned int j=0;j<strlen(flag_list);j++) {
				if (*(flag_list+j) == ' ') *(flag_list+j)='_';
			}
			ptr = strtok_int(NULL, ',', &saveptr1);
			_llLogger()->WriteNextLine(LOG_INFO,"Mod flag: %s",flag_list);
			_llUtils()->AddFlag(flag_list);
		}
	}


	return 1;
}

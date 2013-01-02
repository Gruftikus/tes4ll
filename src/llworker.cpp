
#include "..\include\llworker.h"
#include "..\include\llutils.h"
#include "..\include\lllogger.h"

//constructor
llWorker::llWorker() {

	command_name = NULL;

	name.resize(0);
	flag.resize(0);
	i_value.resize(0);
	f_value.resize(0);
	d_value.resize(0);
	s_value.resize(0);

}

int llWorker::RegisterFlag(char *_name, int *_flag, int _opt) {
	AddElement();
	name[GetSize()-1] = _name;
	flag[GetSize()-1] = _flag;
	opt[GetSize()-1]  = _opt;
	return 1;
}

int llWorker::RegisterValue(char *_name, int *_value, int _opt) {
	AddElement();
	name[GetSize()-1]    = _name;
	i_value[GetSize()-1] = _value;
	opt[GetSize()-1]     = _opt;
	return 1;
}

int llWorker::RegisterValue(char *_name, float *_value, int _opt) {
	AddElement();
	name[GetSize()-1]    = _name;
	f_value[GetSize()-1] = _value;
	opt[GetSize()-1]     = _opt;
	return 1;
}

int llWorker::RegisterValue(char *_name, double *_value, int _opt) {
	AddElement();
	name[GetSize()-1]    = _name;
	d_value[GetSize()-1] = _value;
	opt[GetSize()-1]     = _opt;
	return 1;
}

int llWorker::RegisterValue(char *_name, char **_value, int _opt) {
	AddElement();
	name[GetSize()-1]    = _name;
	s_value[GetSize()-1] = _value;
	opt[GetSize()-1]     = _opt;
	return 1;
}

int llWorker::CheckFlag (char *_flag) {
	for (unsigned int i=0; i<name.size(); i++) {
		if (_stricmp(_flag, name[i]) == 0) {
			if (flag[i]) {
				used[i] = 1;
				*(flag[i]) = 1;
				return 1;
			}
		}
	}
	return 0;
}

int llWorker::CheckValue(char *_value) {
	for (unsigned int i=0; i<name.size(); i++) {
		if (_stricmp(_value, name[i]) == 0) {
			used[i] = 1;
			if (i_value[i]) {
				checked_value = 1;
				checked_pos   = i;
				return 1;
			} else if (f_value[i]) {
				checked_value = 2;
				checked_pos   = i;
				return 2;
			} else if (d_value[i]) {
				checked_value = 3;
				checked_pos   = i;
				return 3;
			} else if (s_value[i]) {
				checked_value = 4;
				checked_pos   = i;
				return 3;
			} 
		}
	}
	return 0;
}

int llWorker::AddValue(char *_value) {
	if (checked_value == 1) {
		char *dummy = new char[strlen(_value)+1];
		strcpy_s(dummy, strlen(_value)+1, _value);
		_llUtils()->StripQuot(&dummy);
		sscanf_s(dummy, "%i", i_value[checked_pos]);
		delete dummy;
	} else if (checked_value == 2) {
		char *dummy = new char[strlen(_value)+1];
		strcpy_s(dummy, strlen(_value)+1, _value);
		_llUtils()->StripQuot(&dummy);
		sscanf_s(dummy, "%f", f_value[checked_pos]);
		delete dummy;
	} else if (checked_value == 3) {
		char *dummy = new char[strlen(_value)+1];
		strcpy_s(dummy, strlen(_value)+1, _value);
		_llUtils()->StripQuot(&dummy);
		sscanf_s(dummy, "%lf", d_value[checked_pos]);
		delete dummy;
	} else if (checked_value == 4) {
		char *dummy = new char[strlen(_value)+1];
		strcpy_s(dummy, strlen(_value)+1, _value);
		_llUtils()->StripQuot(&dummy);
		*(s_value[checked_pos]) = dummy;
	} 
	return checked_value;
}

int llWorker::Used(char *_flag) {
	for (unsigned int i=0; i<name.size(); i++) {
		if (_stricmp(_flag, name[i]) == 0) {
			return used[i];
		}
	}
	return 0;
}

int llWorker::RegisterOptions(void) {
	return 1;
}

int llWorker::Prepare(void) {
	for (unsigned int i=0; i<name.size(); i++) {
		used[i] = 0;
	}
	return 1;
}

int llWorker::Init(void) {
	for (unsigned int i=0; i<name.size(); i++) {
		if ((opt[i] & LLWORKER_OBL_OPTION) && used[i] == 0) {
			_llLogger()->WriteNextLine(-LOG_WARNING,"%s: obligatory option [%s] missing", command_name, name[i]);
			return 0;
		}
	}
	return 1;
}

void llWorker::Print(void) {

	_llLogger()->WriteNextLine(LOG_COMMAND,"%s ", GetCommandName());

	for (unsigned int i=0; i<name.size(); i++) {
		if (used[i]) {
			if (i_value[i]) {
				_llLogger()->AddToLine(' ');
				_llLogger()->AddToLine(name[i]);
				_llLogger()->AddToLine("=", *(i_value[i]));
			} else if (f_value[i]) {
				_llLogger()->AddToLine(' ');
				_llLogger()->AddToLine(name[i]);
				_llLogger()->AddToLine("=", *(f_value[i]), 0);
			} else if (d_value[i]) {
				_llLogger()->AddToLine(' ');
				_llLogger()->AddToLine(name[i]);
				_llLogger()->AddToLine("=", *(d_value[i]), 0);
			} else if (s_value[i] && *s_value[i]) {
				_llLogger()->AddToLine(' ');
				_llLogger()->AddToLine(name[i]);
				_llLogger()->AddToLine("=\"");
				_llLogger()->AddToLine(*(s_value[i]));
				_llLogger()->AddToLine('\"');
			} else if (flag[i]) {
				_llLogger()->AddToLine(' ');
				_llLogger()->AddToLine(name[i]);
			} 
		}
	}
	_llLogger()->Dump();
}
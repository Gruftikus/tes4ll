#ifndef _PLLWORKER_H_
#define _PLLWORKER_H_

#define LLWORKER_OBL_OPTION 1

#include <iostream>
#include <vector>

class llWorker {

 protected:

	 char *command_name;
	 int checked_value, checked_pos;

	 std::vector<char*>   name;
	 std::vector<int*>    flag;
	 std::vector<int*>    i_value;
	 std::vector<float*>  f_value;
	 std::vector<double*> d_value;
	 std::vector<char**>  s_value;
	 std::vector<int>     opt;
	 std::vector<int>     used;

	 void AddElement(void) {
		 int size = GetSize();
		 name.resize(size + 1);
		 name[size] = NULL;
		 flag.resize(size + 1);
		 flag[size] = NULL;
		 i_value.resize(size + 1);
		 i_value[size] = NULL;
		 f_value.resize(size + 1);
		 f_value[size] = NULL;
		 d_value.resize(size + 1);
		 d_value[size] = NULL;
		 s_value.resize(size + 1);
		 s_value[size] = NULL;
		 opt.resize(size + 1);
		 opt[size] = 0;
		 used.resize(size + 1);
	 };

 public:

    llWorker();

	void SetCommandName(char *_name) {
		command_name = _name;
	};

	char *GetCommandName(void) {
		return command_name;
	}

    int RegisterFlag( char *_name, int    *_flag,  int _opt = 0);
	int RegisterValue(char *_name, int    *_value, int _opt = 0);
	int RegisterValue(char *_name, float  *_value, int _opt = 0);
	int RegisterValue(char *_name, double *_value, int _opt = 0);
	int RegisterValue(char *_name, char   **_value, int _opt = 0);

	int GetSize(void) {
		return name.size();
	}

	virtual llWorker * Clone() {
		return new llWorker(*this);
	}

	int CheckFlag (char *_flag);
	int CheckValue(char *_value);
	int AddValue  (char *_value);

	int SetValue (char *_name, char *_value) {
		//std::cout << _name << std::endl;
		if (!CheckValue(_name)) return 0;
		//std::cout << _value << std::endl;
		if (!AddValue(_value)) return 0;
		return 1;
	};

	int Used(char *_flag);

	virtual int RegisterOptions(void);
	virtual int Prepare(void);
	virtual int Init(void);

	virtual void Print(void);

};

#endif

#include "pch.h"
#include "stdafx.h"
#include "Processor.h"
#include <tchar.h>

//+------------------------------------------------------------------+
//| Constructor                                                      |
//+------------------------------------------------------------------+
Processor::Processor() {
}
//+------------------------------------------------------------------+
//| Destructor                                                       |
//+------------------------------------------------------------------+
Processor::~Processor() {
}
//+------------------------------------------------------------------+
//| Run Program                                                      |
//+------------------------------------------------------------------+
int Processor::Run() {
	string file = processor.file_path + processor.front_name + "-" + processor.back_name;

	string in_file = file + ".in";
	string out_file = file + ".out";

	while (1) {
		// Read file .in .out get value
		if (processor.ReadFile(out_file))
			return 1;
		if (processor.ReadFile(in_file))
			return 1;
	}
	return 0;
}
//+------------------------------------------------------------------+
//| Read File                                                        |
//+------------------------------------------------------------------+
int Processor::ReadFile(string input) {
	fstream myfile(input.c_str(), fstream::in);
	string line;
	size_t   p = 0;
	myfile.seekg(p);
	if (myfile.is_open()) {
		while (myfile.eof() == false) {
			getline(myfile, line);
			if (!input.compare(input.size() - 3, 3, ".in")) {
				if (FindField(line, "35=8") > -1) {
					cout << line.substr(0, FindField(line, "8=") - 3) << " | ";
					cout << line.substr(FindField(line, "11=") + 3, FindField(line, "453=") - FindField(line, "11=") - 4) << endl;
				}
			}
			else {
				if (FindField(line, "35=D") > -1) {
					cout << line.substr(0, FindField(line, "8=") - 3) << " | ";
					cout << line.substr(FindField(line, "11=") + 3, FindField(line, "453=") - FindField(line, "11=") - 4) << endl;
				}
			}
		}
		p = myfile.tellg();  //*2
		LOGI << "ReadFile Success " << input;
		return 0;
	}
	else {
		LOGE << "Cannot read " << input;
		return 1;
	}
}
//+------------------------------------------------------------------+
//| Find Field B2bits                                                |
//+------------------------------------------------------------------+
int Processor::FindField(string line, char* input) {
	char *tmp = new char[line.size() + 1];
	copy(line.begin(), line.end(), tmp);
	char *result = strstr(tmp, input);
	int position = result - tmp;

	return position;
}
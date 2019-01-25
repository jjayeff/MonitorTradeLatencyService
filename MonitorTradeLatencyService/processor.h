#pragma once
#include "stdafx.h"
#include <plog/vnlog.h>

class Processor
{
public:
	Processor();
	~Processor();

public:
	struct FileOut
	{
		string				id = "";						// 11= ClOrdID 
		string				time = "";						// Run time
		string				msg_type = "";
		string				account = "";
		string				group = "";
	};
	struct FileIn
	{
		string				id = "";						// 11= ClOrdID 
		string				time = "";						// Run time
	};
	struct Data
	{
		string				id = "";						// 11= ClOrdID 
		string				diftime = "";					// different time
		string				account = "";
		string				group = "";
		string				msg_type = "";
	};
	vector<FileOut>		out_file;
	vector<FileIn>		in_file;
	LogClass			vnLog;
	string				front_name = "";
	string				back_name = "";
	string				key_front_name = "";
	string				key_back_name = "";
	string				file_path = "";
	string				result_path = "";
	int					diff = 0;
	int					deley = 0;
	int					file_in_line = 0;
	int					file_out_line = 0;

private:

public:
	int					Run();
	int					ReadFile(string input);
	int					FindField(string line, char* input);
	int					WriteFile();
	string				DiffTime(string time1, string time2);
	string				Diff2String(float time);
	int					SetFrontBackName();

private:
};

extern Processor processor;
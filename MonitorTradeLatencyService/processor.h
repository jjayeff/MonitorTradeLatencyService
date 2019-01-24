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
		string				account = "";
		string				group = "";
		bool				check = false;
	};
	struct FileIn
	{
		string				id = "";						// 11= ClOrdID 
		string				time = "";						// Run time
		bool				check = false;
	};
	struct Data
	{
		string				id = "";						// 11= ClOrdID 
		string				diftime = "";					// different time
		string				account = "";
		string				group = "";
	};
	vector<FileOut>		out_file;
	vector<FileIn>		in_file;
	vector<Data>		data;
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
	void				writeConfig(LPCTSTR path, LPCTSTR key, string value);
	int					SetFrontBackName();

private:
};

extern Processor processor;
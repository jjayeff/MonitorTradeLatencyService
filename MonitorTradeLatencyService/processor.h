#pragma once
#include "stdafx.h"
#include <plog/vnlog.h>

class Processor
{
public:
	Processor();
	~Processor();

public:
	struct File
	{
		string				id = "";						// 11= ClOrdID 
		string				log_time = "";						// Run time
		string				msg_type = "";
		string				account = "";
		string				group = "";
		string				sending_time = "";
		string				transact_time = "";
		string				order_type = "";
	};
	struct Data
	{
		string				id = "";						// 11= ClOrdID 
		string				diftime = "";					// different time
		string				account = "";
		string				group = "";
		string				order_type = "";
	};
	vector<File>		MF_out;
	vector<File>		MF_in;
	vector<File>		out_acc_file[9];
	vector<File>		in_acc_file[9];
	vector<Data>		data;
	vector<string>		account;
	LogClass			vnLog;
	string				front_name = "";
	string				back_name = "";
	string				front_acc_name = "";
	string				back_acc_name = "";
	string				key_front_name = "";
	string				key_back_name = "";
	string				file_path = "";
	string				result_path = "";
	int					diff = 0;
	int					deley = 0;
	int					file_in_line = 0;
	int					file_out_line = 0;
	int					file_in_acc_line[8] = { 0 };
	int					file_out_acc_line[8] = { 0 };

private:

public:
	int					Run();
	int					ReadFile(string input);
	int					ReadFile(string input, vector<File> *value, int index);
	int					FindField(string line, char* input);
	int					WriteFile();
	int					WriteAverageFile();
	string				DiffTime(string time1, string time2);
	string				PushTime(string time, float diff);
	string				Diff2String(float time);
	float				String2Diff(string difftime);
	int					SetFrontBackName();
	int					SetFrontBackName(string key_front, string key_back);
	int					CutString(string input);
	string				CutStringGroup(string input);

private:
};

extern Processor processor;
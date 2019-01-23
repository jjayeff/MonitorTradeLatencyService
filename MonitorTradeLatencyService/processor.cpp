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
	if (SetFrontBackName())
		return 1;

	string file = file_path + front_name + "-" + back_name;
	LOGI << "Read file name: " + front_name + "-" + back_name;
	string in_file = file + ".in";
	string out_file = file + ".out";

	// Read file .in .out get value
	if (ReadFile(out_file))
		return 1;
	if (ReadFile(in_file))
		return 1;

	if (WriteFile())
		return 1;

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
		int count = 0;
		while (myfile.eof() == false) {
			getline(myfile, line);
			if (!input.compare(input.size() - 3, 3, ".in")) {
				if (FindField(line, "35=8") > -1 && count > file_in_line) {
					FileIn tmp;
					tmp.id = line.substr(FindField(line, "11=") + 3, FindField(line, "453=") - FindField(line, "11=") - 4);
					tmp.time = line.substr(0, FindField(line, "8=") - 3);
					in_file.push_back(tmp);
					file_in_line = count;
				}
			}
			else {
				if (FindField(line, "35=D") > -1 && count > file_out_line) {
					FileOut tmp;
					tmp.id = line.substr(FindField(line, "11=") + 3, FindField(line, "453=") - FindField(line, "11=") - 4);
					tmp.time = line.substr(0, FindField(line, "8=") - 3);
					tmp.account = line.substr(FindField(line, "452=") + 9, FindField(line, "581=") - FindField(line, "452=") - 10);
					tmp.group = line.substr(FindField(line, "50001=") + 6, FindField(line, "50002=") - FindField(line, "50001=") - 7);
					out_file.push_back(tmp);
					file_out_line = count;
				}
			}
			count++;
		}
		p = myfile.tellg();  //*2
		//LOGI << "ReadFile Success " << input;
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
//+------------------------------------------------------------------+
//| Write File                                                       |
//+------------------------------------------------------------------+
int Processor::WriteFile() {
	if (out_file.size() < 1 || in_file.size() < 1)
		return 0;
	else {
		time_t t = time(0);   // get time now
		tm* now = localtime(&t);
		string date = to_string(now->tm_year + 1900) + to_string(now->tm_mon + 1) + to_string(now->tm_mday);
		ofstream mywrite(result_path + "MTLS-" + date + ".txt");

		for (int i = 0; i < out_file.size(); i++)
			for (int j = 0; j < in_file.size(); j++)
				if (out_file[i].id == in_file[j].id && !out_file[i].check && !in_file[j].check) {
					Data tmp;
					tmp.id = out_file[i].id;
					tmp.diftime = DiffTime(out_file[i].time, in_file[j].time);
					tmp.account = out_file[i].account;
					tmp.group = out_file[i].group;
					data.push_back(tmp);
					in_file[j].check = true;
					out_file[i].check = true;
					LOGI << "Success Data: " << tmp.id << " | Difftime: " << tmp.diftime;
					break;
				}

		for (int i = 0; i < data.size(); i++)
			mywrite << data[i].account << "," << data[i].group << "," << data[i].id << "," << data[i].diftime << "\n";

		return 0;
	}
}
//+------------------------------------------------------------------+
//| Cal different time                                               |
//+------------------------------------------------------------------+
string Processor::DiffTime(string time1, string time2) {
	time_t tStart;
	int ymd1, hh1, mm1, ss1, mss1, ymd2, hh2, mm2, ss2, mss2;
	const char *start_time = time1.c_str();
	const char *end_time = time2.c_str();

	sscanf(start_time, "%d-%d:%d:%d.%d", &ymd1, &hh1, &mm1, &ss1, &mss1);
	sscanf(end_time, "%d-%d:%d:%d.%d", &ymd2, &hh2, &mm2, &ss2, &mss2);

	string start = to_string(ss1) + "." + to_string(mss1);
	string end = to_string(ss2) + "." + to_string(mss2);

	return to_string(stof(end) - stof(start));
}
//+------------------------------------------------------------------+
//| Other Function                                                   |
//+------------------------------------------------------------------+
void Processor::writeConfig(LPCTSTR path, LPCTSTR key, string value) {
	LPCTSTR result = value.c_str();
	WritePrivateProfileString(_T("Application"), key, result, path);
}
int Processor::SetFrontBackName() {
	string path = file_path;
	string real_path = path;
	string max_string = "";
	double max = -999999;

	char *cstr_front_name = new char[key_front_name.length()];
	strcpy(cstr_front_name, key_front_name.c_str());
	char *cstr_back_name = new char[key_back_name.length()];
	strcpy(cstr_back_name, key_back_name.c_str());
	for (const auto & entry : fs::directory_iterator(path)) {
		ostringstream oss;
		oss << entry;
		string path = oss.str();
		int index_front = FindField(path, cstr_front_name);
		int index_back = FindField(path, cstr_back_name);
		if ((path.substr(path.size() - 3, 3) == ".in" || path.substr(path.size() - 4, 4) == ".out")
			&& FindField(path, cstr_front_name) > -1
			&& processor.FindField(path, cstr_back_name) > -1
			&& (path.substr(path.size() - 7, 7) != ".ndx.in" && path.substr(path.size() - 8, 8) != ".ndx.out")
			) {
			// Get Modified time
			char *tmp = new char[path.length()];
			strcpy(tmp, path.c_str());
			struct stat fileInfo;
			if (stat(tmp, &fileInfo) != 0) {  // Use stat( ) to get the info
				LOGE << "Error: " << strerror(errno);
				return 1;
			}
			// Compare last time of file
			time_t t = time(0);   // get time now
			double seconds = difftime(fileInfo.st_ctime, t);
			if (path.substr(path.size() - 3, 3) == ".in") {
				if (max < seconds || max_string < path.substr(index_back + key_back_name.length() + 1, path.length() - index_back - key_back_name.length() - 4)) {
					max_string = path.substr(index_back + key_back_name.length() + 1, path.length() - index_back - key_back_name.length() - 4);
					max = seconds;
					real_path = path.substr(0, path.size() - 3);
				}
			}
			else {
				if (max < seconds || max_string < path.substr(index_back + key_back_name.length() + 1, path.length() - index_back - key_back_name.length() - 5)) {
					max_string = path.substr(index_back + key_back_name.length() + 1, path.length() - index_back - key_back_name.length() - 5);
					max = seconds;
					real_path = path.substr(0, path.size() - 4);
				}
			}
		}
	}
	if (real_path == "./") {
		LOGE << "Cannot find file please set key front and back name";
		return 1;
	}

	int index_front = FindField(real_path, cstr_front_name);
	int index_back = FindField(real_path, cstr_back_name);
	front_name = real_path.substr(index_front, index_back - index_front - 1);
	back_name = real_path.substr(index_back, real_path.size());
	writeConfig(".\\MonitorTradeLatencyService.ini", "FrontName", real_path.substr(index_front, index_back - index_front - 1));
	writeConfig(".\\MonitorTradeLatencyService.ini", "BackName", real_path.substr(index_back, real_path.size()));

	return 0;
}
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
	if (!SetFrontBackName()) {
		string file = file_path + front_name + "-" + back_name;
		LOGI << "Read file name: " + front_name + "-" + back_name;
		string read_in_file = file + ".in";
		string read_out_file = file + ".out";

		// Read file .in .out get value
		if (ReadFile(read_out_file))
			return 1;
		if (ReadFile(read_in_file))
			return 1;
		if (WriteFile())
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
		int count = 0;
		while (myfile.eof() == false) {
			getline(myfile, line); 
			if (!input.compare(input.size() - 3, 3, ".in")) {
				if (FindField(line, "35=8") > -1 && count > file_in_line ) {
					FileIn tmp;
					tmp.id = line.substr(FindField(line, "11=") + 3, 20);
					tmp.time = line.substr(0, FindField(line, "8=") - 3);
					for (int i = 0; i < in_file.size(); i++)
						if (in_file[i].id == tmp.id) {
							in_file[i].time = tmp.time;
							break;
						}
						else if (i + 1 == in_file.size()) {
							in_file.push_back(tmp);
						}
					if (in_file.size() == 0)
						in_file.push_back(tmp);
					file_in_line = count;
				}
			}
			else {
				if ((FindField(line, "35=D") > -1 || FindField(line, "35=F") > -1 || FindField(line, "35=G") > -1) && count > file_out_line) {
					FileOut tmp;
					tmp.id = line.substr(FindField(line, "11=") + 3, 20);
					tmp.time = line.substr(0, FindField(line, "8=") - 3);
					if (FindField(line, "35=D") > -1) {
						if (FindField(line, "452=") > -1)
							tmp.account = line.substr(FindField(line, "452=") + 9, FindField(line, "581=") - FindField(line, "452=") - 10);
						else
							tmp.account = "NONE";
					}
					else {
						if (FindField(line, "11=") > -1)
							tmp.account = line.substr(FindField(line, "11=") + 26, FindField(line, "581=") - FindField(line, "11=") - 27);
						else
							tmp.account = "NONE";
					}
					tmp.group = line.substr(FindField(line, "50001=") + 6, FindField(line, "50002=") - FindField(line, "50001=") - 7);
					tmp.msg_type = line.substr(FindField(line, "35=") + 3, 1);
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
		ofstream mywrite(result_path + "MonitorTradeLatencyService-" + date + ".csv", std::ofstream::out | std::ofstream::app);

		for (int i = 0; i < out_file.size(); i++)
			for (int j = 0; j < in_file.size(); j++)
				if (out_file[i].id == in_file[j].id) {
					Data tmp;
					tmp.id = out_file[i].id;
					tmp.diftime = DiffTime(out_file[i].time, in_file[j].time);
					tmp.account = out_file[i].account;
					tmp.group = out_file[i].group;
					tmp.msg_type = out_file[i].msg_type;
					mywrite << tmp.account << "," << tmp.group << "," << tmp.id << "," << Diff2String(stof(tmp.diftime)) << "," << tmp.msg_type << "," << "\n";
					if (diff * 60 < stof(tmp.diftime)) {
						LOGW << "Diff Over: " << tmp.id << " | Difftime: " << tmp.diftime;
					}
					else
						LOGI << "Success Data: " << tmp.id << " | Difftime: " << tmp.diftime;
					out_file.erase(out_file.begin() + i--);
					in_file.erase(in_file.begin() + j);
					break;
				}

		/*float sum = 0;
		vector<string>groups;
		vector<string>accounts;
		for (int i = 0; i < data.size(); i++) {
			sum += stof(data[i].diftime);

			if (groups.size() == 0)
				groups.push_back(data[i].group);
			if (accounts.size() == 0)
				accounts.push_back(data[i].account);

			for (int j = 0; j < groups.size(); j++)
				if (data[i].group == groups[j])
					break;
				else if (j + 1 == groups.size())
					groups.push_back(data[i].group);

			for (int j = 0; j < accounts.size(); j++)
				if (data[i].account == accounts[j])
					break;
				else if (j + 1 == accounts.size())
					accounts.push_back(data[i].account);
		}

		// Average of all
		mywrite << "\nAverage: " << Diff2String(sum / data.size()) << "\n";
		mywrite << "\n";

		// Average by group
		for (int i = 0; i < groups.size(); i++) {
			float sum = 0;
			int count = 0;
			for (int j = 0; j < data.size(); j++) {
				if (groups[i] == data[j].group) {
					sum += stof(data[j].diftime);
					count++;
				}
			}
			mywrite << "Average(" << groups[i] << "): " << Diff2String(sum / count) << "\n";
		}
		mywrite << "\n";

		// Average by account
		for (int i = 0; i < accounts.size(); i++) {
			float sum = 0;
			int count = 0;
			for (int j = 0; j < data.size(); j++) {
				if (accounts[i] == data[j].account) {
					sum += stof(data[j].diftime);
					count++;
				}
			}
			mywrite << "Average(" << accounts[i] << "): " << Diff2String(sum / count) << "\n";
		}*/

		mywrite.close();

		return 0;
	}
}
//+------------------------------------------------------------------+
//| Cal different time                                               |
//+------------------------------------------------------------------+
string Processor::DiffTime(string time1, string time2) {
	time_t tStart;
	int ymd1, hh1, mm1, ymd2, hh2, mm2;
	float ss1, ss2;
	const char *start_time = time1.c_str();
	const char *end_time = time2.c_str();

	sscanf(start_time, "%d-%d:%d:%f", &ymd1, &hh1, &mm1, &ss1);
	sscanf(end_time, "%d-%d:%d:%f", &ymd2, &hh2, &mm2, &ss2);
	float strat = hh1 * 60 * 60 + mm1 * 60 + ss1;
	float end = hh2 * 60 * 60 + mm2 * 60 + ss2;
	return to_string(end - strat);
}
//+------------------------------------------------------------------+
//| Diff to String                                                   |
//+------------------------------------------------------------------+
string Processor::Diff2String(float difftime) {
	string tmp = "";
	if (difftime > 3600) {
		tmp += to_string((int)difftime / 3600) + ":";
		difftime -= (3600 * ((int)difftime / 3600));
	}
	else
		tmp += "00:";
	if (difftime > 60) {
		tmp += to_string((int)difftime / 60) + ":";
		difftime -= (60 * ((int)difftime / 60));
	}
	else
		tmp += "00:";
	if (difftime < 60)
		tmp += to_string(difftime);
	return tmp;
}
//+------------------------------------------------------------------+
//| Other Function                                                   |
//+------------------------------------------------------------------+
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
	if (real_path == path) {
		LOGE << "Cannot find file please set key front and back name";
		return 1;
	}

	int index_front = FindField(real_path, cstr_front_name);
	int index_back = FindField(real_path, cstr_back_name);
	front_name = real_path.substr(index_front, index_back - index_front - 1);
	back_name = real_path.substr(index_back, real_path.size());

	return 0;
}
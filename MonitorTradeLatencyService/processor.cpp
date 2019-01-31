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


		// Read file .in .out get account value
		for (int i = 0; i < account.size(); i++) {
			int run = 1;
			run = SetFrontBackName(account[i], "SET");
			if (!run) {
				file = file_path + front_acc_name + "-" + back_acc_name;
				read_in_file = file + ".in";
				read_out_file = file + ".out";
				if (ReadFile(read_out_file, &out_acc_file[i], i))
					return 1;
				if (ReadFile(read_in_file, &in_acc_file[i], i))
					return 1;
			}
		}

		// Write result
		if (WriteFile())
			return 1;
		if (WriteAverageFile())
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
			if (FindField(line, "35=8") > -1 && count > file_in_line && FindField(line, "150=0") > -1) {
				File tmp;
				tmp.id = line.substr(FindField(line, "11=") + 3, 20);
				tmp.time = line.substr(0, FindField(line, "8=") - 3);
				tmp.sending_time = line.substr(FindField(line, "52=") + 3, 20);
				tmp.transact_time = line.substr(FindField(line, "60=") + 3, 20);
				if (line.substr(FindField(line, "54=") + 3, 1) == "1")
					tmp.order_type = "Buy/";
				else if (line.substr(FindField(line, "54=") + 3, 1) == "2")
					tmp.order_type = "Sell/";
				if (line.substr(FindField(line, "40=") + 3, 1) == "1")
					tmp.order_type += "MP";
				else if (line.substr(FindField(line, "40=") + 3, 1) == "2")
					tmp.order_type += "Limit";
				for (int i = 0; i < in_file.size(); i++)
					if (in_file[i].id == tmp.id) {
						break;
					}
					else if (i + 1 == in_file.size()) {
						in_file.push_back(tmp);
					}
				if (in_file.size() == 0)
					in_file.push_back(tmp);
				file_in_line = count;
			}
			else if ((FindField(line, "35=D") > -1 || FindField(line, "35=F") > -1 || FindField(line, "35=G") > -1) && count > file_out_line) {
				File tmp;
				tmp.id = line.substr(FindField(line, "11=") + 3, 20);
				tmp.time = line.substr(0, FindField(line, "8=") - 3);
				tmp.sending_time = line.substr(FindField(line, "52=") + 3, 20);
				tmp.transact_time = line.substr(FindField(line, "60=") + 3, 20);
				if (line.substr(FindField(line, "54=") + 3, 1) == "1")
					tmp.order_type = "Buy/";
				else if (line.substr(FindField(line, "54=") + 3, 1) == "2")
					tmp.order_type = "Sell/";
				if (FindField(line, "40=") > -1) {
					if (line.substr(FindField(line, "40=") + 3, 1) == "1")
						tmp.order_type += "MP";
					else if (line.substr(FindField(line, "40=") + 3, 1) == "2")
						tmp.order_type += "Limit";
				}
				else
					tmp.order_type += "Cancel";
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
int Processor::ReadFile(string input, vector<File> *value, int index) {
	fstream myfile(input.c_str(), fstream::in);
	string line;
	size_t   p = 0;
	myfile.seekg(p);
	if (myfile.is_open()) {
		int count = 0;
		while (myfile.eof() == false) {
			getline(myfile, line);
			if (FindField(line, "35=8") > -1 && count > file_in_acc_line[index]) {
				File tmp;
				tmp.id = line.substr(FindField(line, "11=") + 3, 20);
				tmp.time = line.substr(0, FindField(line, "8=") - 3);
				tmp.sending_time = line.substr(FindField(line, "52=") + 3, 20);
				for (int i = 0; i < value->size(); i++)
					if ((*value)[i].id == tmp.id) {
						break;
					}
					else if (i + 1 == value->size()) {
						value->push_back(tmp);
					}
				if (value->size() == 0)
					value->push_back(tmp);
				file_in_acc_line[index] = count;
			}
			else if ((FindField(line, "35=D") > -1 || FindField(line, "35=F") > -1 || FindField(line, "35=G") > -1) && count > file_out_acc_line[index]) {
				File tmp;
				tmp.id = line.substr(FindField(line, "11=") + 3, 20);
				tmp.time = line.substr(0, FindField(line, "8=") - 3);
				tmp.sending_time = line.substr(FindField(line, "52=") + 3, 20);
				tmp.transact_time = line.substr(FindField(line, "60=") + 3, 20);
				value->push_back(tmp);
				file_out_acc_line[index] = count;
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
		ofstream mywrite(result_path + "MonitorTradeLatencyService_" + date + ".csv", std::ofstream::out | std::ofstream::app);
		ofstream mywrite1(result_path + "MonitorTradeLatencyService-Sub_" + date + ".csv", std::ofstream::out | std::ofstream::app);

		for (int i = 0; i < out_file.size(); i++) {
			for (int j = 0; j < in_file.size(); j++)
				if (out_file[i].id == in_file[j].id) {
					Data tmp;
					tmp.id = out_file[i].id;
					tmp.diftime = DiffTime(out_file[i].time, in_file[j].time);
					tmp.account = out_file[i].account;
					tmp.group = out_file[i].group;
					tmp.order_type = out_file[i].order_type;
					vector<string> time_out_acc, time_in_acc;
					int tmp1 = -1, tmp2 = -1, index = -1;
					for (int k = 0; k < account.size(); k++)
						if (tmp.account.substr(0, 2) == account[k].substr(3, 2))
							index = k;
						else if (tmp.account.substr(0, 2) == "97" && account[k].substr(2, 3) == "117")
							index = k;
						else if (tmp.account.substr(0, 2) == "98" && account[k].substr(2, 3) == "118")
							index = k;
					if (index > -1) {
						for (int k = 0; k < out_acc_file[index].size(); k++)
							if (out_file[i].id == out_acc_file[index][k].id) {
								time_out_acc.push_back(out_acc_file[index][k].time);
								time_out_acc.push_back(out_acc_file[index][k].sending_time);
								time_out_acc.push_back(out_acc_file[index][k].transact_time);
								tmp1 = k;
								break;
							}
						for (int k = 0; k < in_acc_file[index].size(); k++)
							if (out_file[i].id == in_acc_file[index][k].id) {
								time_in_acc.push_back(in_acc_file[index][k].time);
								time_in_acc.push_back(in_acc_file[index][k].sending_time);
								time_in_acc.push_back(in_acc_file[index][k].transact_time);
								tmp2 = k;
								break;
							}
					}
					if (tmp1 > -1 && tmp2 > -1) {
						float T1 = stof(DiffTime(out_file[i].time, time_out_acc[0]));
						float T2 = stof(DiffTime(time_out_acc[0], time_in_acc[0]));
						float T3 = stof(DiffTime(time_in_acc[0], in_file[j].time));
						/*float t1 = stof(DiffTime(out_file[i].time, out_file[i].transact_time));
						float t2 = stof(DiffTime(out_file[i].transact_time, out_file[i].sending_time));
						float t3 = stof(DiffTime(out_file[i].sending_time, time_out_acc[0]));
						float t5 = stof(DiffTime(time_out_acc[0], time_in_acc[0]));
						float t6 = stof(DiffTime(time_in_acc[0], in_file[j].time));
						float t7 = stof(DiffTime(in_file[j].time, in_file[j].transact_time));
						cout << "===================================" << endl;
						cout << tmp.id << "," << Diff2String(stof(tmp.diftime)) << "," << Diff2String(T1) << "," << Diff2String(T2) << "," << Diff2String(T3) << "\n";
						cout << "===================================" << endl;
						cout << "Dm: " << out_file[i].time << " | " << out_file[i].sending_time << " | " << out_file[i].transact_time << endl;
						cout << "Ds: " << time_out_acc[0] << " | " << time_out_acc[1] << " | " << time_out_acc[2] << endl;
						cout << "8s: " << time_in_acc[0] << " | " << time_in_acc[1] << " | " << time_in_acc[2] << endl;
						cout << "8m: " << in_file[j].time << " | " << in_file[j].sending_time << " | " << in_file[j].transact_time << endl;
						cout << "===================================" << endl;
						cout << "t1: " << out_file[i].transact_time.substr(15, 10) << " - " << out_file[i].time.substr(15, 10) << " = " << t1 << endl;
						cout << "t2: " << out_file[i].sending_time.substr(15, 10) << " - " << out_file[i].transact_time.substr(15, 10) << " = " << t2 << endl;
						cout << "t3: " << time_out_acc[1].substr(15, 10) << " - " << out_file[i].sending_time.substr(15, 10) << " = " << t3 << endl;
						cout << "t5: " << time_in_acc[0].substr(15, 10) << " - " << time_out_acc[0].substr(15, 10) << " = " << t5 << endl;
						cout << "t6: " << in_file[j].time.substr(15, 10) << " - " << time_in_acc[0].substr(15, 10) << " = " << t6<< endl;
						cout << "t7: " << in_file[j].transact_time.substr(15, 10) << " - " << in_file[j].time.substr(15, 10) << " = " << t7 << endl;
						cout << "===================================" << endl;*/
						mywrite1 << tmp.id << "," << Diff2String(stof(tmp.diftime)) << "," << Diff2String(T1) << "," << Diff2String(T2) << "," << Diff2String(T3) << "\n";
						mywrite << tmp.order_type << "," << tmp.account << "," << tmp.group << "," << tmp.id << "," << Diff2String(stof(tmp.diftime)) << "," << "\n";
						if (diff < stof(tmp.diftime)) {
							LOGW << "Diff Over: " << tmp.id << " | Difftime: " << tmp.diftime;
						}
						else
							LOGI << "Success Data: " << tmp.id << " | Difftime: " << tmp.diftime;
						// Delete from memory
						out_file.erase(out_file.begin() + i--);
						in_file.erase(in_file.begin() + j);
						out_acc_file[index].erase(out_acc_file[index].begin() + tmp1);
						in_acc_file[index].erase(in_acc_file[index].begin() + tmp2);
						break;
					}
				}
		}

		mywrite.close();
		mywrite1.close();

		return 0;
	}
}
int Processor::WriteAverageFile() {
	time_t t = time(0);   // get time now
	tm* now = localtime(&t);
	string date = to_string(now->tm_year + 1900) + to_string(now->tm_mon + 1) + to_string(now->tm_mday);
	ofstream mywrite(result_path + "MonitorTradeLatencyService-Average_" + date + ".csv");

	fstream myfile(result_path + "MonitorTradeLatencyService_" + date + ".csv", fstream::in);
	Data tmp;
	size_t   p = 0;
	myfile.seekg(p);
	if (myfile.is_open()) {
		while (myfile.eof() == false) {
			while (getline(myfile, tmp.order_type, ',')) {
				getline(myfile, tmp.account, ',');
				getline(myfile, tmp.group, ',');
				getline(myfile, tmp.id, ',');
				getline(myfile, tmp.diftime);
				data.push_back(tmp);
			}
		}
	}
	else {
		LOGE << "Cannot read " << result_path + "MonitorTradeLatencyService_" + date + ".csv";
		return 1;
	}

	float sum = 0, buy_mp = 0, sell_mp = 0, buy_limit = 0, sell_limit = 0;
	int count_bm = 0, count_sm = 0, count_bl = 0, count_sl = 0;
	vector<string>groups;
	vector<string>accounts;
	for (int i = 0; i < data.size(); i++) {
		sum += String2Diff(data[i].diftime);
		if (data[i].order_type == "Buy/MP") {
			buy_mp += String2Diff(data[i].diftime);
			count_bm++;
		}
		else if (data[i].order_type == "Sell/MP") {
			sell_mp += String2Diff(data[i].diftime);
			count_sm++;
		}
		else if (data[i].order_type == "Buy/Limit") {
			buy_limit += String2Diff(data[i].diftime);
			count_bl++;
		}
		else if (data[i].order_type == "Sell/Limit") {
			sell_limit += String2Diff(data[i].diftime);
			count_sl++;
		}
		if (groups.size() == 0)
			groups.push_back(CutStringGroup(data[i].group));
		if (accounts.size() == 0)
			accounts.push_back(data[i].account);

		for (int j = 0; j < groups.size(); j++)
			if (CutStringGroup(data[i].group) == groups[j])
				break;
			else if (j + 1 == groups.size())
				groups.push_back(CutStringGroup(data[i].group));

		for (int j = 0; j < accounts.size(); j++)
			if (data[i].account == accounts[j])
				break;
			else if (j + 1 == accounts.size())
				accounts.push_back(data[i].account);
	}

	// Average of all
	mywrite << "============================ Average ============================" << "\n";
	mywrite << "Average: " << Diff2String(sum / data.size()) << "\n";
	mywrite << "- Average(Buy/MP): " << Diff2String(buy_mp / count_bm) << "\n";
	mywrite << "- Average(Sell/MP): " << Diff2String(sell_mp / count_sm) << "\n";
	mywrite << "- Average(Buy/Limit): " << Diff2String(buy_limit / count_bl) << "\n";
	mywrite << "- Average(Sell/Limit): " << Diff2String(sell_limit / count_sl) << "\n";

	// Average by group
	mywrite << "\n========================= Group Average =========================" << "\n";
	for (int i = 0; i < groups.size(); i++) {
		float sum = 0, buy_mp = 0, sell_mp = 0, buy_limit = 0, sell_limit = 0;
		int count = 0, count_bm = 0, count_sm = 0, count_bl = 0, count_sl = 0;
		for (int j = 0; j < data.size(); j++) {
			if (groups[i] == CutStringGroup(data[j].group)) {
				sum += String2Diff(data[j].diftime);
				if (data[j].order_type == "Buy/MP") {
					buy_mp += String2Diff(data[j].diftime);
					count_bm++;
				}
				else if (data[j].order_type == "Sell/MP") {
					sell_mp += String2Diff(data[j].diftime);
					count_sm++;
				}
				else if (data[j].order_type == "Buy/Limit") {
					buy_limit += String2Diff(data[j].diftime);
					count_bl++;
				}
				else if (data[j].order_type == "Sell/Limit") {
					sell_limit += String2Diff(data[j].diftime);
					count_sl++;
				}
				count++;
			}
		}
		mywrite << "Average(" << groups[i] << "): " << Diff2String(sum / count) << "\n";
		mywrite << "- Average(" << groups[i] << " Buy/MP): " << Diff2String(buy_mp / count_bm) << "\n";
		mywrite << "- Average(" << groups[i] << " Sell/MP): " << Diff2String(sell_mp / count_sm) << "\n";
		mywrite << "- Average(" << groups[i] << " Buy/Limit): " << Diff2String(buy_limit / count_bl) << "\n";
		mywrite << "- Average(" << groups[i] << " Sell/Limit): " << Diff2String(sell_limit / count_sl) << "\n";
	}

	// Average by account
	mywrite << "\n======================== Account Average ========================" << "\n";
	for (int i = 0; i < accounts.size(); i++) {
		float sum = 0, buy_mp = 0, sell_mp = 0, buy_limit = 0, sell_limit = 0;
		int count = 0, count_bm = 0, count_sm = 0, count_bl = 0, count_sl = 0;
		for (int j = 0; j < data.size(); j++) {
			if (accounts[i] == data[j].account) {
				sum += String2Diff(data[j].diftime);
				if (data[j].order_type == "Buy/MP") {
					buy_mp += String2Diff(data[j].diftime);
					count_bm++;
				}
				else if (data[j].order_type == "Sell/MP") {
					sell_mp += String2Diff(data[j].diftime);
					count_sm++;
				}
				else if (data[j].order_type == "Buy/Limit") {
					buy_limit += String2Diff(data[j].diftime);
					count_bl++;
				}
				else if (data[j].order_type == "Sell/Limit") {
					sell_limit += String2Diff(data[j].diftime);
					count_sl++;
				}
				count++;
			}
		}
		mywrite << "Average(" << accounts[i] << "): " << Diff2String(sum / count) << "\n";
		mywrite << "- Average(" << accounts[i] << " Buy/MP): " << Diff2String(buy_mp / count_bm) << "\n";
		mywrite << "- Average(" << accounts[i] << " Sell/MP): " << Diff2String(sell_mp / count_sm) << "\n";
		mywrite << "- Average(" << accounts[i] << " Buy/Limit): " << Diff2String(buy_limit / count_bl) << "\n";
		mywrite << "- Average(" << accounts[i] << " Sell/Limit): " << Diff2String(sell_limit / count_sl) << "\n";
	}

	mywrite << "\n=================================================================" << "\n";
	mywrite.close();

	// Clear memory
	groups.clear();
	accounts.clear();
	data.clear();

	return 0;
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
float Processor::String2Diff(string difftime) {
	time_t tStart;
	int  hh, mm;
	float ss;
	const char *diff_time = difftime.c_str();
	sscanf(diff_time, "%d:%d:%f", &hh, &mm, &ss);
	return ss;
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
		LOGW << "Cannot find file please set key front and back name";
		return 1;
	}

	int index_front = FindField(real_path, cstr_front_name);
	int index_back = FindField(real_path, cstr_back_name);
	front_name = real_path.substr(index_front, key_front_name.length());
	back_name = real_path.substr(index_back, real_path.size());

	return 0;
}
int Processor::SetFrontBackName(string key_front, string key_back) {
	string path = file_path;
	string real_path = path;
	string max_string = "";
	double max = -999999;

	char *cstr_front_name = new char[key_front.length()];
	strcpy(cstr_front_name, key_front.c_str());
	char *cstr_back_name = new char[key_back.length()];
	strcpy(cstr_back_name, key_back.c_str());
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
		LOGW << "Cannot find file :" + key_front + "-" + key_back;
		return 1;
	}

	int index_front = FindField(real_path, cstr_front_name);
	int index_back = FindField(real_path, cstr_back_name);
	front_acc_name = real_path.substr(index_front, key_front.length());
	back_acc_name = real_path.substr(index_back, real_path.size());

	return 0;
}
int Processor::CutString(string input) {
	string tmp = "";
	for (int i = 0; i < input.length(); i++) {
		if (input[i] == ',') {
			account.push_back(tmp);
			tmp = "";
		}
		if ((input[i] == ',' && input[i + 1] == ' '))
			i++;
		else if (input[i] == ' ' && input[i - 1] == ',')
			i++;
		else if (input[i] != ',')
			tmp += input[i];
	}
	return 0;
}
string Processor::CutStringGroup(string input) {
	string tmp;
	for (int i = 0; i < input.length(); i++)
		if (input[i] == '_')
			return tmp;
		else
			tmp += input[i];
	return tmp;
}
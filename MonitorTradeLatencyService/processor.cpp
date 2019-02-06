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
	CreateDirectory(tmp_results_path.c_str(), NULL);
	time_t t = time(0);   // get time now
	tm* now = localtime(&t);
	string mon = now->tm_mon < 10 + 1 ? "0" + to_string(now->tm_mon + 1) : to_string(now->tm_mon + 1);
	string day = now->tm_mday < 10 ? "0" + to_string(now->tm_mday) : to_string(now->tm_mday);
	string date = to_string(now->tm_year + 1900) + mon + day;
	if (date_config != date) {
		string tmp_path = tmp_results_path + "\\MonitorTradeLatencyService" + date + "\\";
		CreateDirectory(tmp_path.c_str(), NULL);
		ofstream mywrite(processor.result_path + "MonitorTradeLatencyService" + date + "\\" + "MonitorTradeLatencyService_" + date + ".csv");
		ofstream mywrite1(processor.result_path + "MonitorTradeLatencyService" + date + "\\" + "MonitorTradeLatencyService_" + date + "_Sub.csv");
		mywrite.close();
		mywrite1.close();
		date_config = date;
		writeConfig(".\\MonitorTradeLatencyService.ini", "Date", date);
	}

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
		// Connect Database
		if (ConnectDataBase())
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
//| Database Function                                                |
//+------------------------------------------------------------------+
int Processor::ConnectDataBase() {
	// Connect Datebase
	if (!dbs.connect(db_driver, db_server, db_user, db_password))
	{
		LOGE << "!Database connect fail";
		dbs.commit();
		return 1;
	}
	else
	{
		LOGI << "Database connected : " << db_server << ", Driver : " << db_driver;
		return 0;
	}
}
int Processor::SendEmail(Data value, float *time) {
	if (!dbs.isConnected())
	{
		LOGE << "Database disconnect! Try reconnect!";
		dbs.connect();
	}

	char cmd_temp[512];
	sprintf_s(cmd_temp, "INSERT INTO acc_info.dbo.send_email(broker_id, mt4_login_id, due_date, template_email, email_to, email_from, email_subject, param1, param2, param3, param4, param5, param6, param7, param8, param9, param10) VALUES('%s', '%d', GETDATE(), '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s');",
		CutStringGroup(value.group).c_str(),
		stoi(value.account.c_str()),
		email_template.c_str(),
		email_to.c_str(),
		email_from.c_str(),
		"[MonitorTradeLatency] Over time",
		value.id.c_str(),
		value.diftime.c_str(),
		value.order_type.c_str(),
		to_string(time[0]).c_str(),
		to_string(time[1]).c_str(),
		to_string(time[2]).c_str(),
		to_string(time[3]).c_str(),
		to_string(time[4]).c_str(),
		to_string(time[5]).c_str(),
		to_string(time[6]).c_str());

	if (!dbs.execute(cmd_temp))
	{
		LOGE << "!Excute database fail";
		return 0;
	}

	LOGI << "SendEmail Success!!";
	return 1;
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
				tmp.log_time = line.substr(0, FindField(line, "8=") - 3);
				tmp.sending_time = line.substr(FindField(line, "52=") + 3, FindField(line, "369=") - FindField(line, "52=") - 4);
				tmp.transact_time = line.substr(FindField(line, "60=") + 3, FindField(line, "77=") - FindField(line, "60=") - 4);
				if (line.substr(FindField(line, "54=") + 3, 1) == "1")
					tmp.order_type = "Buy/";
				else if (line.substr(FindField(line, "54=") + 3, 1) == "2")
					tmp.order_type = "Sell/";
				if (line.substr(FindField(line, "40=") + 3, 1) == "1")
					tmp.order_type += "MP";
				else if (line.substr(FindField(line, "40=") + 3, 1) == "2")
					tmp.order_type += "Limit";
				for (int i = 0; i < MF_in.size(); i++)
					if (MF_in[i].id == tmp.id) {
						break;
					}
					else if (i + 1 == MF_in.size()) {
						MF_in.push_back(tmp);
					}
				if (MF_in.size() == 0)
					MF_in.push_back(tmp);
				file_in_line = count;
			}
			else if ((FindField(line, "35=D") > -1 || FindField(line, "35=F") > -1 || FindField(line, "35=G") > -1) && count > file_out_line) {
				File tmp;
				tmp.id = line.substr(FindField(line, "11=") + 3, 20);
				tmp.log_time = line.substr(0, FindField(line, "8=") - 3);
				tmp.sending_time = line.substr(FindField(line, "52=") + 3, FindField(line, "11=") - FindField(line, "52=") - 4);
				tmp.transact_time = line.substr(FindField(line, "60=") + 3, FindField(line, "38=") - FindField(line, "60=") - 4);
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
				MF_out.push_back(tmp);
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
				tmp.log_time = line.substr(0, FindField(line, "8=") - 3);
				tmp.sending_time = line.substr(FindField(line, "52=") + 3, FindField(line, "369=") - FindField(line, "52=") - 4);
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
				tmp.log_time = line.substr(0, FindField(line, "8=") - 3);
				tmp.sending_time = line.substr(FindField(line, "52=") + 3, FindField(line, "11=") - FindField(line, "52=") - 4);
				tmp.transact_time = line.substr(FindField(line, "60=") + 3, FindField(line, "38=") - FindField(line, "60=") - 4);
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
	char *tmp = &line[0u];
	copy(line.begin(), line.end(), tmp);
	char *result = strstr(tmp, input);
	int position = result - tmp;

	return position;
}
//+------------------------------------------------------------------+
//| Write File                                                       |
//+------------------------------------------------------------------+
int Processor::WriteFile() {
	if (MF_out.size() < 1 || MF_in.size() < 1)
		return 0;
	else {
		time_t t = time(0);   // get time now
		tm* now = localtime(&t);
		string mon = now->tm_mon < 10 + 1 ? "0" + to_string(now->tm_mon + 1) : to_string(now->tm_mon + 1);
		string day = now->tm_mday < 10 ? "0" + to_string(now->tm_mday) : to_string(now->tm_mday);
		string date = to_string(now->tm_year + 1900) + mon + day;
		ofstream mywrite(result_path + "MonitorTradeLatencyService" + date + "\\" + "MonitorTradeLatencyService_" + date + ".csv", std::ofstream::out | std::ofstream::app);
		ofstream mywrite1(result_path + "MonitorTradeLatencyService" + date + "\\" + "MonitorTradeLatencyService_" + date + "_Sub.csv", std::ofstream::out | std::ofstream::app);

		for (int i = 0; i < MF_out.size(); i++) {
			for (int j = 0; j < MF_in.size(); j++)
				if (MF_out[i].id == MF_in[j].id) {
					Data tmp;
					tmp.id = MF_out[i].id;
					tmp.diftime = DiffTime(MF_in[j].log_time, MF_out[i].log_time);
					tmp.account = MF_out[i].account;
					tmp.group = MF_out[i].group;
					tmp.order_type = MF_out[i].order_type;
					File FS_out, FS_in;
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
							if (MF_out[i].id == out_acc_file[index][k].id) {
								FS_out.log_time = out_acc_file[index][k].log_time;
								FS_out.sending_time = out_acc_file[index][k].sending_time;
								FS_out.transact_time = out_acc_file[index][k].transact_time;
								tmp1 = k;
								break;
							}
						for (int k = 0; k < in_acc_file[index].size(); k++)
							if (MF_out[i].id == in_acc_file[index][k].id) {
								FS_in.log_time = in_acc_file[index][k].log_time;
								FS_in.sending_time = in_acc_file[index][k].sending_time;
								FS_in.transact_time = in_acc_file[index][k].transact_time;
								tmp2 = k;
								break;
							}
					}
					if (tmp1 > -1 && tmp2 > -1) {
						float T1 = 0, T2 = 0, T3 = 0, T4 = 0, T5 = 0, T6 = 0, T7 = 0, T8 = 0, TALL = 0;
						float FS_diff = 0;
						string Y = "", X = "";
						fixed;
						setprecision(3);
						FS_diff = stof(DiffTime(FS_in.sending_time, FS_in.log_time));
						Y = PushTime(FS_out.sending_time, FS_diff);
						X = PushTime(FS_in.log_time, FS_diff);
						T1 = stof(DiffTime(FS_out.log_time, MF_out[i].log_time));
						T2 = stof(DiffTime(FS_out.sending_time, FS_out.log_time));
						T3 = stof(DiffTime(MF_in[j].transact_time, Y));
						T4 = stof(DiffTime(FS_in.sending_time, MF_in[j].transact_time));
						T5 = stof(DiffTime(X, FS_in.sending_time));
						T6 = stof(DiffTime(MF_in[j].sending_time, MF_in[j].log_time));
						T7 = stof(DiffTime(MF_in[j].log_time, FS_in.log_time));
						TALL = stof(DiffTime(FS_in.log_time, FS_out.log_time));
						mywrite1 << tmp.id << "," << Diff2String(T1) << "," << Diff2String(T2) << "," << Diff2String(T3) << "," << Diff2String(T4) << "," << Diff2String(T5) << "," << Diff2String(T6) << "," << Diff2String(T7) << "\n";
						mywrite << tmp.order_type << "," << tmp.account << "," << tmp.group << "," << tmp.id << "," << Diff2String(stof(tmp.diftime)) << "," << "\n";
						if (diff < stof(tmp.diftime)) {
							float t_array[7] = { T1, T2, T3, T4, T5, T6, T7 };
							SendEmail(tmp, t_array);
							LOGW << "Diff Over: " << tmp.id << " | Difftime: " << tmp.diftime;
						}
						else
							LOGI << "Success Data: " << tmp.id << " | Difftime: " << tmp.diftime;
						// Delete from memory
						MF_out.erase(MF_out.begin() + i--);
						MF_in.erase(MF_in.begin() + j);
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
	string mon = now->tm_mon < 10 + 1 ? "0" + to_string(now->tm_mon + 1) : to_string(now->tm_mon + 1);
	string day = now->tm_mday < 10 ? "0" + to_string(now->tm_mday) : to_string(now->tm_mday);
	string date = to_string(now->tm_year + 1900) + mon + day;
	ofstream mywrite(result_path + "MonitorTradeLatencyService" + date + "\\" + "MonitorTradeLatencyService_" + date + "_Average.csv");

	fstream myfile(result_path + "MonitorTradeLatencyService" + date + "\\" + "MonitorTradeLatencyService_" + date + ".csv", fstream::in);
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
				data_file.push_back(tmp);
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
	for (int i = 0; i < data_file.size(); i++) {
		sum += String2Diff(data_file[i].diftime);
		if (data_file[i].order_type == "Buy/MP") {
			buy_mp += String2Diff(data_file[i].diftime);
			count_bm++;
		}
		else if (data_file[i].order_type == "Sell/MP") {
			sell_mp += String2Diff(data_file[i].diftime);
			count_sm++;
		}
		else if (data_file[i].order_type == "Buy/Limit") {
			buy_limit += String2Diff(data_file[i].diftime);
			count_bl++;
		}
		else if (data_file[i].order_type == "Sell/Limit") {
			sell_limit += String2Diff(data_file[i].diftime);
			count_sl++;
		}
		if (groups.size() == 0)
			groups.push_back(CutStringGroup(data_file[i].group));
		if (accounts.size() == 0)
			accounts.push_back(data_file[i].account);

		for (int j = 0; j < groups.size(); j++)
			if (CutStringGroup(data_file[i].group) == groups[j])
				break;
			else if (j + 1 == groups.size())
				groups.push_back(CutStringGroup(data_file[i].group));

		for (int j = 0; j < accounts.size(); j++)
			if (data_file[i].account == accounts[j])
				break;
			else if (j + 1 == accounts.size())
				accounts.push_back(data_file[i].account);
	}

	// Average of all
	mywrite << "============================ Average ============================" << "\n";
	mywrite << "Average: " << Diff2String(sum / data_file.size()) << "\n";
	mywrite << "- Average(Buy/MP): " << Diff2String(buy_mp / count_bm) << "\n";
	mywrite << "- Average(Sell/MP): " << Diff2String(sell_mp / count_sm) << "\n";
	mywrite << "- Average(Buy/Limit): " << Diff2String(buy_limit / count_bl) << "\n";
	mywrite << "- Average(Sell/Limit): " << Diff2String(sell_limit / count_sl) << "\n";

	// Average by group
	mywrite << "\n========================= Group Average =========================" << "\n";
	for (int i = 0; i < groups.size(); i++) {
		float sum = 0, buy_mp = 0, sell_mp = 0, buy_limit = 0, sell_limit = 0;
		int count = 0, count_bm = 0, count_sm = 0, count_bl = 0, count_sl = 0;
		for (int j = 0; j < data_file.size(); j++) {
			if (groups[i] == CutStringGroup(data_file[j].group)) {
				sum += String2Diff(data_file[j].diftime);
				if (data_file[j].order_type == "Buy/MP") {
					buy_mp += String2Diff(data_file[j].diftime);
					count_bm++;
				}
				else if (data_file[j].order_type == "Sell/MP") {
					sell_mp += String2Diff(data_file[j].diftime);
					count_sm++;
				}
				else if (data_file[j].order_type == "Buy/Limit") {
					buy_limit += String2Diff(data_file[j].diftime);
					count_bl++;
				}
				else if (data_file[j].order_type == "Sell/Limit") {
					sell_limit += String2Diff(data_file[j].diftime);
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
		for (int j = 0; j < data_file.size(); j++) {
			if (accounts[i] == data_file[j].account) {
				sum += String2Diff(data_file[j].diftime);
				if (data_file[j].order_type == "Buy/MP") {
					buy_mp += String2Diff(data_file[j].diftime);
					count_bm++;
				}
				else if (data_file[j].order_type == "Sell/MP") {
					sell_mp += String2Diff(data_file[j].diftime);
					count_sm++;
				}
				else if (data_file[j].order_type == "Buy/Limit") {
					buy_limit += String2Diff(data_file[j].diftime);
					count_bl++;
				}
				else if (data_file[j].order_type == "Sell/Limit") {
					sell_limit += String2Diff(data_file[j].diftime);
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
	data_file.clear();

	return 0;
}
//+------------------------------------------------------------------+
//| Cal different time                                               |
//+------------------------------------------------------------------+
float round(float y, int n) {
	float x = abs(y);
	int d = 0;
	if ((x * pow(10, n + 1)) - (floorf(x * pow(10, n))) > 4) d = 1;
	x = (floorf(x * pow(10, n))) / pow(10, n);
	if (y > -1)
		return x;
	else
		return x * -1;
}
string Processor::DiffTime(string time1, string time2) {
	time_t tStart;
	int ymd1, hh1, mm1, ymd2, hh2, mm2;
	float ss1, ss2;
	const char *start_time = time2.c_str();
	const char *end_time = time1.c_str();

	sscanf(start_time, "%d-%d:%d:%f", &ymd1, &hh1, &mm1, &ss1);
	sscanf(end_time, "%d-%d:%d:%f", &ymd2, &hh2, &mm2, &ss2);
	float strat = hh1 * 60 * 60 + mm1 * 60 + ss1;
	float end = hh2 * 60 * 60 + mm2 * 60 + ss2;
	return to_string(round((end - strat), 3));
}
string Processor::PushTime(string time, float diff) {
	time_t tStart;
	int ymd1, hh1, mm1, ymd2, hh2, mm2;
	float ss1, ss2;
	const char *start_time = time.c_str();

	sscanf(start_time, "%d-%d:%d:%f", &ymd1, &hh1, &mm1, &ss1);
	float result = (hh1 * 60 * 60 + mm1 * 60 + (ss1 + diff));
	string tmp = "";
	if (result > 3600) {
		tmp += to_string((int)result / 3600) + ":";
		result -= (3600 * ((int)result / 3600));
	}
	else
		tmp += "00:";
	if (result > 60) {
		tmp += to_string((int)result / 60) + ":";
		result -= (60 * ((int)result / 60));
	}
	else
		tmp += "00:";
	if (result < 60)
		tmp += to_string(result);
	return to_string(ymd1) + "-" + tmp;
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

	char *cstr_front_name = &key_front_name[0u];
	strcpy(cstr_front_name, key_front_name.c_str());
	char *cstr_back_name = &key_back_name[0u];
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
			char *tmp = &path[0u];
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

	char *cstr_front_name = &key_front[0u];
	strcpy(cstr_front_name, key_front.c_str());
	char *cstr_back_name = &key_back[0u];
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
			char *tmp = &path[0u];
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
string Processor::GetIpByName(string hostname)
{
	string ans = hostname;
	WSADATA wsaData;
	int iResult;
	DWORD dwError;

	struct hostent* remoteHost;
	struct in_addr addr;

	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0)
	{
		return hostname;
	}

	remoteHost = gethostbyname(ans.c_str());

	if (remoteHost == NULL)
	{
		dwError = WSAGetLastError();
		if (dwError != 0)
		{
			if (dwError == WSAHOST_NOT_FOUND)
			{
				return ans;
			}
			else if (dwError == WSANO_DATA)
			{
				return ans;
			}
			else
			{
				return ans;
			}
		}
	}
	else
	{
		if (remoteHost->h_addrtype == AF_INET)
		{
			addr.s_addr = *(u_long *)remoteHost->h_addr_list[0];
			ans = inet_ntoa(addr);
		}
	}
	return ans;
}
void Processor::writeConfig(LPCTSTR path, LPCTSTR key, string value) {
	LPCTSTR result = value.c_str();
	WritePrivateProfileString(_T("Application"), key, result, path);
}
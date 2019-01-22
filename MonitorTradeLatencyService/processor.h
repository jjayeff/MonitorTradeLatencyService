#pragma once
#include "stdafx.h"
#include <plog/vnlog.h>

class Processor
{
public:
	Processor();
	~Processor();

public:
	LogClass			vnLog;
	string				front_name = "";
	string				back_name = "";
	string				file_path = "";

private:

public:
	int					Run();
	int					ReadFile(string input);
	int					FindField(string line, char* input);

private:
};

extern Processor processor;
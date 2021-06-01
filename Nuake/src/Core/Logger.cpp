#include "Logger.h"
#include <iostream>
#include <chrono>
#include <string>
#include <time.h>

std::vector <std::string> Logger::logs = std::vector<std::string>();

void Logger::Log(std::string log)
{
	char buff[100];
	time_t now = time(0);
	strftime(buff, 100, "%Y-%m-%d %H:%M:%S.000", localtime(&now));
	std::string msg = "["+ std::string(buff) + "]" + std::string(" - ") + log;
	printf((msg + "\n").c_str());
	logs.push_back(msg);
}

std::vector<std::string> Logger::GetLogs()
{
	return logs;
}
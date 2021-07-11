#include "Logger.h"
#include <iostream>
#include <chrono>
#include <string>
#include <time.h>

namespace Nuake
{
	std::vector<LogEntry>  Logger::m_Logs = std::vector<LogEntry>();

	void Logger::Log(std::string log, LOG_TYPE type)
	{
		char buff[100];
		time_t now = time(0);
		strftime(buff, 100, "%Y-%m-%d %H:%M:%S.000", localtime(&now));

		LogEntry newLog = {
			type,
			buff,
			log
		};

		std::string msg = "[" + std::string(buff) + "]" + std::string(" - ") + log;
		printf((msg + "\n").c_str());

		if (m_Logs.size() >= MAX_LOG)
			m_Logs.erase(m_Logs.begin());

		m_Logs.push_back(newLog);
	}

	std::vector<LogEntry> Logger::GetLogs()
	{
		return m_Logs;
	}
}

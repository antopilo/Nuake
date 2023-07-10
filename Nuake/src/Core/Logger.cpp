#include "Logger.h"

#include <iostream>
#include <chrono>
#include <string>
#include <time.h>

namespace Nuake
{
	std::vector<LogEntry> Logger::m_Logs = std::vector<LogEntry>();

	void Logger::Log(const std::string& log, LOG_TYPE type)
	{
		char buff[100];
		time_t now = time(0);
		struct tm timeinfo;
		localtime_s(&timeinfo, &now);
		strftime(buff, 100, "%H:%M:%S", &timeinfo);

		LogEntry newLog = {
			type,
			buff,
			log
		};

		std::string msg = "[" + std::string(buff) + "]" + std::string(" - ") + log;
		std::cout << msg << std::endl;

		if (m_Logs.size() >= MAX_LOG)
			m_Logs.erase(m_Logs.begin());

		m_Logs.push_back(newLog);
	}

	std::vector<LogEntry> Logger::GetLogs()
	{
		return m_Logs;
	}
}

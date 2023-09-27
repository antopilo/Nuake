#include "Logger.h"

#include <iostream>
#include <chrono>
#include <string>
#include <time.h>

namespace Nuake
{
	std::vector<LogEntry> Logger::m_Logs = std::vector<LogEntry>();

	void Logger::Log(const std::string& log, const std::string& logger ,LOG_TYPE type)
	{
		char buff[100];
		time_t now = time(0);
		struct tm timeinfo;

#ifdef NK_WIN
		
		localtime_s(&timeinfo, &now);
		strftime(buff, 100, "%H:%M:%S", &timeinfo);
#endif

#ifdef NK_LINUX
		time_t timeinfo2;
		localtime_r(&timeinfo2, &timeinfo);
		strftime(buff, 100, "%H:%M:%S", localtime(&now));
#endif

		LogEntry newLog = {
			type,
			buff,
			log,
			logger
		};

		switch (type)
		{
		case WARNING:
			// TODO: Add color
			break;
		case CRITICAL:

			break;
		case VERBOSE:

			break;
		}

		std::string msg = "[" + std::string(buff) + "] " + logger + " - " + log;
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

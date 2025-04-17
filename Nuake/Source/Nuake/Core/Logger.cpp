#include "Logger.h"

#include <iostream>
#include <chrono>
#include <string>
#include <time.h>

namespace Nuake
{
	std::vector<LogEntry> Logger::m_Logs = std::vector<LogEntry>();

	void Logger::Log(const std::string& log, const std::string& logger, LOG_TYPE type)
	{
		if (!m_Logs.empty() && m_Logs.back().message == log)
		{
			m_Logs.back().count += 1;
			return;
		}

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
			logger,
			0
		};

		std::string color;
		switch (type)
		{
		case WARNING:
			color = "\033[1;33m";
			// TODO: Add color
			break;
		case COMPILATION:
			color = "\033[1;34m";
			break;
		case CRITICAL:
			color = "\033[1;31m";
			break;
		case VERBOSE:
			color = "\033[0m";
			break;
		}
		
		// Icons
		std::string transformedLogger = logger;
		if (logger == "window")
		{
			transformedLogger += reinterpret_cast<const char*>(u8"🪟");
		}

		std::string msg = color + buff;

		if (!logger.empty())
		{
			msg += " [" + transformedLogger + "]";
		}

		msg += " \033[38;5;245m" + log;

		std::cout << msg << std::endl;

		// Simulating m_Logs push logic
		// Assuming m_Logs is a std::vector<std::string>
		static std::vector<std::string> m_Logs;
		constexpr size_t MAX_LOG = 1000;
		if (m_Logs.size() >= MAX_LOG)
			m_Logs.erase(m_Logs.begin());

		m_Logs.push_back(msg); // Use msg or whatever your final log content is
	}

	std::vector<LogEntry>& Logger::GetLogs()
	{
		return m_Logs;
	}

	void Logger::ClearLogs()
	{
		m_Logs.clear();
	}
}

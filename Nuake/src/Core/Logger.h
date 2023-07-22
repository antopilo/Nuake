#pragma once
#include <string>
#include <vector>

namespace Nuake
{
	enum LOG_TYPE
	{
		VERBOSE,
		WARNING,
		CRITICAL
	};

	struct LogEntry
	{
		LOG_TYPE type;
		std::string time;
		std::string message;
		std::string logger;
	};

	class Logger
	{
	private:
		static const int MAX_LOG = 64;
		static std::vector<LogEntry> m_Logs;

	public:
		static void Log(const std::string& log, const std::string& logger = "main", LOG_TYPE type = VERBOSE);
		static std::vector<LogEntry> GetLogs();
	};
}

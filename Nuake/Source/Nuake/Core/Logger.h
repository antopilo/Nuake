#pragma once
#include <string>
#include <vector>

namespace Nuake
{
	enum LOG_TYPE
	{
		VERBOSE,
		WARNING,
		CRITICAL,
		COMPILATION
	};

	struct LogEntry
	{
		LOG_TYPE type;
		std::string time;
		std::string message;
		std::string logger;
		uint32_t count;
	};

	class Logger
	{
	private:
		static const int MAX_LOG = 64;
		static std::vector<LogEntry> m_Logs;

	public:
		static void Log(const std::string& log, const std::string& logger = "main", LOG_TYPE type = VERBOSE);
		static std::vector<LogEntry> GetLogs();
		static void ClearLogs();
		static size_t GetLogCount() { return m_Logs.size(); }
	};
}

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
	};

	class Logger
	{
	public:
		static void Log(std::string log, LOG_TYPE type = VERBOSE);
		static std::vector<LogEntry> GetLogs();
	private:
		static const int MAX_LOG = 64;
		static std::vector<LogEntry> m_Logs; // TODO: Use log struct.
	};
}

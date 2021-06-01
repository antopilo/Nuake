#pragma once
#include <string>
#include <vector>

enum LOG_TYPE
{
	VERBOSE,
	WARNING,
	CRITICAL
};

struct Log
{
	LOG_TYPE type;
	std::string time;
	std::string message;
};

class Logger
{
	const int MAX_LOG = 64;
	static std::vector<std::string> logs;
	static std::vector<Log> m_Logs; // TODO: Use log struct.
public:
	static void Log(std::string log);
	static std::vector<std::string> GetLogs();
};
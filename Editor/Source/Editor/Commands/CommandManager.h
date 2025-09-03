#pragma once
#include <string>

class CommandManager
{
public:
	static CommandManager& Get()
	{
		static CommandManager instance;
		return instance;
	}

	void Push(const std::string& command, const std::string& value);
	
};
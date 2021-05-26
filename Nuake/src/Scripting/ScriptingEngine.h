#pragma once
#include <string>
#include "sol/forward.hpp"
#include <map>
#include <string>

class ScriptingEngine
{
private:
	static sol::state lua;
	static std::map<std::string, std::string> m_Scripts;

public:
	static void Init();

	static void RegisterScript(std::string path);

	static void InitScript(std::string path);
	static void UpdateScript(std::string path);
	static void ExitScript(std::string path);

	static void RunFile(const std::string& path);
	static void UpdateScript(const std::string& path);
	static void RunCode(const std::string& code);
	static void Close();
};
#include "ScriptingEngine.h"
#include <src/Vendors/sol/sol.hpp>
#include <iostream>
#include <fstream>

inline void my_panic(sol::optional<std::string> maybe_msg) {
	std::cerr << "Lua script error detected!" << std::endl;
	if (maybe_msg) {
		const std::string& msg = maybe_msg.value();
		std::cerr << "\terror message: " << msg << std::endl;
	}
}

sol::state ScriptingEngine::lua = sol::state(sol::c_call<decltype(&my_panic), &my_panic>);

void ScriptingEngine::Init()
{
	lua.open_libraries(sol::lib::base, sol::lib::package);

}

void ScriptingEngine::RunFile(const std::string& path)
{

}

void ScriptingEngine::RunCode(const std::string& code)
{
	lua.script(code);
	bool isfullscreen = lua["config"]["fullscreen"];
	sol::table config = lua["config"];
	float x = lua["config"]["resolution"]["x"];
	return;

}

void ScriptingEngine::UpdateScript(const std::string& path)
{
	std::string fileContent;
	std::string allFile = "";
	// Read from the text file
	std::ifstream MyReadFile("resources/Scripts/test.lua");

	// Use a while loop together with the getline() function to read the file line by line
	while (getline(MyReadFile, fileContent)) {
		allFile.append(fileContent);
	}

	// Close the file
	MyReadFile.close();

	try {
		auto bad_code_result = lua.script(allFile);
		// it did not work
		if (!bad_code_result.valid())
		{
			printf(std::to_string((int)(bad_code_result.status())).c_str());
			return;
		}

		int result = lua["update"]();
	}
	catch (int e)
	{

	}

	


	return;
}

void ScriptingEngine::Close()
{

}
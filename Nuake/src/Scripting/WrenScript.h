#pragma once
#include "ScriptingEngine.h"
#include <map>
#include <string>

class WrenHandle;
class WrenScript
{
public:
	std::map <std::string, WrenHandle*> methods;
	WrenHandle* m_Instance;
	WrenHandle* m_OnInitHandle;
	WrenHandle* m_OnUpdateHandle;
	WrenHandle* m_OnExitHandle;

	WrenScript(const std::string& path, const std::string& mod);

	void CallInit();
	void CallUpdate(float timestep);
	void CallExit();

	void RegisterMethod(const std::string& signature);
	void CallMethod(const std::string& signature);
};
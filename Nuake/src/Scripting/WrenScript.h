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
	WrenHandle* m_OnFixedUpdateHandle;
	WrenHandle* m_OnExitHandle;
	WrenHandle* m_SetEntityIDHandle;

	bool CompiledSuccesfully;

	WrenScript(const std::string& path, const std::string& mod, bool isEntity = false);

	void CallInit();
	void CallUpdate(float timestep);
	void CallFixedUpdate(float timestep);
	void CallExit();

	void RegisterMethod(const std::string& signature);
	void CallMethod(const std::string& signature);

	void SetScriptableEntityID(int id);
};
#pragma once
#include <string>
#include <map>
#include <string>
#include "src/Core/Core.h"
#include "src/Vendors/wren/src/include/wren.hpp"
#include <src/Core/Timestep.h>

class WrenScript;
class ScriptModule;
class ScriptingEngine
{
private:
	static WrenVM* m_WrenVM;
	static std::map<std::string, Ref<WrenScript>> m_Scripts;
	static std::map<std::string, Ref<ScriptModule>> Modules;
public:
	static Ref<WrenScript> RegisterScript(const std::string& path, const std::string& mod);
	static void RegisterModule(Ref<ScriptModule> module);
	static void RegisterMethod(void* method, const std::string& module, const std::string& signature);
	static void InitScript(Ref<WrenScript> script);
	static void UpdateScript(Ref<WrenScript> script, Timestep timestep);
	static void ExitScript(Ref<WrenScript> script);
	static void Init();
	static void RunCode(const std::string& code);
	static void Close();

	static WrenForeignMethodFn bindForeignMethod(
		WrenVM* vm,
		const char* module,
		const char* className,
		bool isStatic,
		const char* signature);

	static WrenVM* GetWrenVM();
};
#pragma once
#include <string>
#include <vector>
#include <map>
#include "src/Core/Core.h"
#include "src/Vendors/wren/src/include/wren.hpp"
#include "src/Core/Timestep.h"

namespace Nuake
{
	class WrenScript;
	class ScriptModule;
	class ScriptingEngine
	{
	private:
		static WrenVM* m_WrenVM;

		static std::vector<std::string> m_LoadedScripts;
		static std::map<std::string, Ref<WrenScript>> m_Scripts;
		static std::map<std::string, Ref<ScriptModule>> Modules;
	public:
		static Ref<WrenScript> RegisterScript(const std::string& path, const std::string& mod);
		static bool IsScriptImported(const std::string& path);
		static void ImportScript(const std::string& path);
		static void RegisterModule(Ref<ScriptModule> module);
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
}

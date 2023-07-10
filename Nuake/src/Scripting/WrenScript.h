#pragma once
#include "ScriptingEngine.h"

#include <map>
#include <string>

struct WrenHandle;

namespace Nuake 
{
	class File;
	class WrenScript
	{
	private:
		bool CompiledSuccesfully;
		bool IsEntity = false;
		std::vector<std::string> mModules;

	public:
		Ref<File> mFile;

		std::map <std::string, WrenHandle*> methods;
		WrenHandle* m_Instance = nullptr;
		WrenHandle* m_OnInitHandle;
		WrenHandle* m_OnUpdateHandle;
		WrenHandle* m_OnFixedUpdateHandle;
		WrenHandle* m_OnExitHandle;
		WrenHandle* m_SetEntityIDHandle;

		// Building
		WrenScript(Ref<File> file, bool isEntity);

		void ParseModules();
		std::vector<std::string> GetModules();
		void Build(unsigned int moduleId, bool isEntity = false);

		// Method calls
		void CallInit();
		void CallUpdate(float timestep);
		void CallFixedUpdate(float timestep);
		void CallExit();

		void RegisterMethod(const std::string& signature);
		void CallMethod(const std::string& signature);

		void SetScriptableEntityID(int id);

		bool HasCompiledSuccesfully() { return CompiledSuccesfully; }
	};
}

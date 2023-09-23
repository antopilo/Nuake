#include "WrenScript.h"

#include "src/Core/Logger.h"
#include "src/Core/String.h"
#include "src/Core/FileSystem.h"

#include <src/Vendors/wren/src/include/wren.h>

namespace Nuake 
{
	WrenScript::WrenScript(Ref<File> file, bool isEntityScript)
	{
		mFile = file;
		m_IsEntityScript = isEntityScript;

		ParseModules();
	}

	void WrenScript::ParseModules()
	{
		std::ifstream MyReadFile(mFile->GetAbsolutePath());
		std::string fileContent = "";

		while (getline(MyReadFile, fileContent))
		{
			bool hasFoundModule = false;

			auto splits = String::Split(fileContent, ' ');
			for (unsigned int i = 0; i < splits.size(); i++)
			{
				std::string s = splits[i];
				if (s == "class" && i + 1 < splits.size() && !hasFoundModule)
				{
					std::string moduleFound = splits[i + 1];
					m_Modules.push_back(moduleFound);
					hasFoundModule = true;
				}
			}
		}

		MyReadFile.close();
	}

	std::vector<std::string> WrenScript::GetModules() const
	{
		return m_Modules;
	}

	void WrenScript::Build(unsigned int moduleId, bool isEntity)
	{
		WrenVM* vm = ScriptingEngine::GetWrenVM();

		const std::string& relativePath = mFile->GetRelativePath();

		// Can't import twice the same script, otherwise gives a compile error.
		if (!ScriptingEngine::IsScriptImported(relativePath))
		{
			const std::string& source = "import \"" + relativePath + "\" for " + GetModules()[moduleId];
			WrenInterpretResult result = wrenInterpret(vm, "main", source.c_str());

			if (result == WREN_RESULT_SUCCESS)
			{
				Logger::Log("[ScriptingEngine] Compiled succesfully: " + std::to_string(result));
				m_HasCompiledSuccesfully = true;
				ScriptingEngine::ImportScript(relativePath);
			}
			else
			{
				Logger::Log("[ScriptingEngine] Compiled failed: " + std::to_string(result));
				m_HasCompiledSuccesfully = false;
				return;
			}
		}

		// Get handle to class
		wrenEnsureSlots(vm, 1);
		wrenGetVariable(vm, "main", GetModules()[moduleId].c_str(), 0);
		WrenHandle* classHandle = wrenGetSlotHandle(vm, 0);

		// Call the constructor
		WrenHandle* constructHandle = wrenMakeCallHandle(vm, "new()");
		wrenCall(vm, constructHandle);

		// Retreive value of constructor
		this->m_Instance = wrenGetSlotHandle(vm, 0);

		// Create handles to the instance methods.
		this->m_OnInitHandle = wrenMakeCallHandle(vm, "init()");
		this->m_OnUpdateHandle = wrenMakeCallHandle(vm, "update(_)");
		this->m_OnFixedUpdateHandle = wrenMakeCallHandle(vm, "fixedUpdate(_)");
		this->m_OnExitHandle = wrenMakeCallHandle(vm, "exit()");

		if (isEntity)
			this->m_SetEntityIDHandle = wrenMakeCallHandle(vm, "SetEntityId(_)");

		m_HasCompiledSuccesfully = true;
	}

	void WrenScript::CallInit()
	{
		if (!m_HasCompiledSuccesfully) return;

		WrenVM* vm = ScriptingEngine::GetWrenVM();
		wrenSetSlotHandle(vm, 0, this->m_Instance);
		WrenInterpretResult result = wrenCall(vm, this->m_OnInitHandle);
	}

	void WrenScript::CallUpdate(float timestep)
	{
		if (!m_HasCompiledSuccesfully) return;

		WrenVM* vm = ScriptingEngine::GetWrenVM();
		wrenEnsureSlots(vm, 2);
		wrenSetSlotHandle(vm, 0, this->m_Instance);
		wrenSetSlotDouble(vm, 1, timestep);
		WrenInterpretResult result = wrenCall(vm, this->m_OnUpdateHandle);
	}

	void WrenScript::CallFixedUpdate(float timestep)
	{
		if (!m_HasCompiledSuccesfully) return;

		WrenVM* vm = ScriptingEngine::GetWrenVM();
		wrenEnsureSlots(vm, 2);
		wrenSetSlotHandle(vm, 0, this->m_Instance);
		wrenSetSlotDouble(vm, 1, timestep);
		WrenInterpretResult result = wrenCall(vm, this->m_OnFixedUpdateHandle);
	}

	void WrenScript::CallExit()
	{
		if (!m_HasCompiledSuccesfully || !m_Instance)
			return;

		WrenVM* vm = ScriptingEngine::GetWrenVM();
		wrenEnsureSlots(vm, 1);
		wrenSetSlotHandle(vm, 0, this->m_Instance);
		WrenInterpretResult result = wrenCall(vm, this->m_OnExitHandle);
	}

	void WrenScript::RegisterMethod(const std::string& signature)
	{
		if (!m_HasCompiledSuccesfully)
			return;

		WrenVM* vm = ScriptingEngine::GetWrenVM();
		WrenHandle* handle = wrenMakeCallHandle(vm, signature.c_str());
		methods.emplace(signature, handle);
	}

	void WrenScript::CallMethod(const std::string& signature)
	{
		if (!m_HasCompiledSuccesfully)
			return;

		WrenVM* vm = ScriptingEngine::GetWrenVM();

		// Not found. maybe try to register it?
		if (methods.find(signature) == methods.end())
			return;

		wrenSetSlotHandle(vm, 0, this->m_Instance);
		WrenHandle* handle = methods[signature];
		WrenInterpretResult result = wrenCall(vm, handle);
	}

	Ref<File> WrenScript::GetFile() const
	{
		return mFile;
	}

	void WrenScript::SetScriptableEntityID(int id)
	{
		if (!m_HasCompiledSuccesfully)
			return;

		WrenVM* vm = ScriptingEngine::GetWrenVM();
		wrenSetSlotHandle(vm, 0, this->m_Instance);
		wrenSetSlotDouble(vm, 1, id);
		WrenInterpretResult result = wrenCall(vm, this->m_SetEntityIDHandle);
	}
}

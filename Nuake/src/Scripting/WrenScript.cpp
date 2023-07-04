#include "WrenScript.h"
#include "src/Core/String.h"

#include <src/Vendors/wren/src/include/wren.h>

namespace Nuake {
	WrenScript::WrenScript(Ref<File> file, bool isEntity)
	{
		mFile = file;
		IsEntity = isEntity;

		ParseModules();
	}

	void WrenScript::ParseModules()
	{
		std::ifstream MyReadFile(mFile->GetAbsolutePath());
		std::string fileContent = "";

		while (getline(MyReadFile, fileContent))
		{
			bool hasFoundModule = false;

			auto& splits = String::Split(fileContent, ' ');
			for (unsigned int i = 0; i < splits.size(); i++)
			{
				std::string s = splits[i];
				if (s == "class" && i + 1 < splits.size() && !hasFoundModule)
				{
					std::string moduleFound = splits[i + 1];
					mModules.push_back(moduleFound);
					hasFoundModule = true;
				}
			}
		}

		// Close the file
		MyReadFile.close();
	}

	std::vector<std::string> WrenScript::GetModules()
	{
		return mModules;
	}

	void WrenScript::Build(unsigned int moduleId, bool isEntity)
	{
		WrenVM* vm = ScriptingEngine::GetWrenVM();

		std::string relativePath = mFile->GetRelativePath();
		// Can't import twice the same script, otherwise gives a compile error.
		if (!ScriptingEngine::IsScriptImported(relativePath))
		{
			std::string source = "import \"" + relativePath + "\" for " + GetModules()[moduleId];
			WrenInterpretResult result = wrenInterpret(vm, "main", source.c_str());

			if (result != WREN_RESULT_SUCCESS)
				CompiledSuccesfully = false;

			if (!CompiledSuccesfully)
				return;

			ScriptingEngine::ImportScript(relativePath);
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

		CompiledSuccesfully = true;
	}

	void WrenScript::CallInit()
	{
		if (!CompiledSuccesfully) return;

		WrenVM* vm = ScriptingEngine::GetWrenVM();
		wrenSetSlotHandle(vm, 0, this->m_Instance);
		WrenInterpretResult result = wrenCall(vm, this->m_OnInitHandle);
	}

	void WrenScript::CallUpdate(float timestep)
	{
		if (!CompiledSuccesfully) return;

		WrenVM* vm = ScriptingEngine::GetWrenVM();
		wrenEnsureSlots(vm, 2);
		wrenSetSlotHandle(vm, 0, this->m_Instance);
		wrenSetSlotDouble(vm, 1, timestep);
		WrenInterpretResult result = wrenCall(vm, this->m_OnUpdateHandle);
	}

	void WrenScript::CallFixedUpdate(float timestep)
	{
		if (!CompiledSuccesfully) return;

		WrenVM* vm = ScriptingEngine::GetWrenVM();
		wrenEnsureSlots(vm, 2);
		wrenSetSlotHandle(vm, 0, this->m_Instance);
		wrenSetSlotDouble(vm, 1, timestep);
		WrenInterpretResult result = wrenCall(vm, this->m_OnFixedUpdateHandle);
	}

	void WrenScript::CallExit()
	{
		if (!CompiledSuccesfully || !m_Instance)
			return;

		WrenVM* vm = ScriptingEngine::GetWrenVM();
		wrenEnsureSlots(vm, 1);
		wrenSetSlotHandle(vm, 0, this->m_Instance);
		WrenInterpretResult result = wrenCall(vm, this->m_OnExitHandle);
	}

	void WrenScript::RegisterMethod(const std::string& signature)
	{
		if (!CompiledSuccesfully)
			return;

		WrenVM* vm = ScriptingEngine::GetWrenVM();
		WrenHandle* handle = wrenMakeCallHandle(vm, signature.c_str());
		methods.emplace(signature, handle);
	}

	void WrenScript::CallMethod(const std::string& signature)
	{
		if (!CompiledSuccesfully)
			return;

		WrenVM* vm = ScriptingEngine::GetWrenVM();

		// Not found. maybe try to register it?
		if (methods.find(signature) == methods.end())
			return;

		wrenSetSlotHandle(vm, 0, this->m_Instance);
		WrenHandle* handle = methods[signature];
		WrenInterpretResult result = wrenCall(vm, handle);
	}

	void WrenScript::SetScriptableEntityID(int id)
	{
		if (!CompiledSuccesfully)
			return;

		WrenVM* vm = ScriptingEngine::GetWrenVM();
		wrenSetSlotHandle(vm, 0, this->m_Instance);
		wrenSetSlotDouble(vm, 1, id);
		WrenInterpretResult result = wrenCall(vm, this->m_SetEntityIDHandle);
	}
}

#include "WrenScript.h"
#include "../Core/FileSystem.h"
#include <src/Vendors/wren/src/include/wren.h>

WrenScript::WrenScript(const std::string& path, const std::string& mod, bool isEntity)
{
	WrenVM* vm = ScriptingEngine::GetWrenVM();

	// Import statement
	std::string source = "import \"" + path + "\" for " + mod;

	CompiledSuccesfully = true;

	// Import file as module
	WrenInterpretResult result = wrenInterpret(vm, "main", source.c_str());
	if (result != WREN_RESULT_SUCCESS)
		CompiledSuccesfully = false;

	if (!CompiledSuccesfully)
		return;

	// Get handle to class
	wrenEnsureSlots(vm, 1);
	wrenGetVariable(vm, "main", mod.c_str(), 0);
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
}

void WrenScript::CallInit()
{
	WrenVM* vm = ScriptingEngine::GetWrenVM();
	wrenSetSlotHandle(vm, 0, this->m_Instance);
	WrenInterpretResult result = wrenCall(vm, this->m_OnInitHandle);
}

void WrenScript::CallUpdate(float timestep)
{
	WrenVM* vm = ScriptingEngine::GetWrenVM();
	wrenEnsureSlots(vm, 2);
	wrenSetSlotHandle(vm, 0, this->m_Instance);
	wrenSetSlotDouble(vm, 1, timestep);
	WrenInterpretResult result = wrenCall(vm, this->m_OnUpdateHandle);
}

void WrenScript::CallFixedUpdate(float timestep)
{
	WrenVM* vm = ScriptingEngine::GetWrenVM();
	wrenEnsureSlots(vm, 2);
	wrenSetSlotHandle(vm, 0, this->m_Instance);
	wrenSetSlotDouble(vm, 1, timestep);
	WrenInterpretResult result = wrenCall(vm, this->m_OnFixedUpdateHandle);
}

void WrenScript::CallExit()
{
	if (!CompiledSuccesfully)
		return;

	WrenVM* vm = ScriptingEngine::GetWrenVM();
	wrenEnsureSlots(vm, 1);
	wrenSetSlotHandle(vm, 0, this->m_Instance);
	WrenInterpretResult result = wrenCall(vm, this->m_OnExitHandle);
}

void WrenScript::RegisterMethod(const std::string& signature)
{
	WrenVM* vm = ScriptingEngine::GetWrenVM();
	WrenHandle* handle =  wrenMakeCallHandle(vm, signature.c_str());
	methods.emplace(signature, handle);
}

void WrenScript::CallMethod(const std::string& signature)
{
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
	WrenVM* vm = ScriptingEngine::GetWrenVM();
	wrenSetSlotHandle(vm, 0, this->m_Instance);
	wrenSetSlotDouble(vm, 1, id);
	WrenInterpretResult result = wrenCall(vm, this->m_SetEntityIDHandle);
}

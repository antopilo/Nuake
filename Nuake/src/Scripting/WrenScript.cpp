#include "WrenScript.h"
#include "../Core/FileSystem.h"
#include <src/Vendors/wren/src/include/wren.h>
WrenScript::WrenScript(const std::string& path, const std::string& mod)
{

	WrenVM* vm = ScriptingEngine::GetWrenVM();

	
	wrenEnsureSlots(vm, 1);
	wrenGetVariable(vm, "main", mod.c_str(), 0);

	this->m_Instance = wrenGetSlotHandle(vm, 0);

	this->m_OnInitHandle = wrenMakeCallHandle(vm, "init()");
	this->m_OnUpdateHandle = wrenMakeCallHandle(vm, "update(_)");
	this->m_OnExitHandle = wrenMakeCallHandle(vm, "exit()");
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

void WrenScript::CallExit()
{
	WrenVM* vm = ScriptingEngine::GetWrenVM();
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

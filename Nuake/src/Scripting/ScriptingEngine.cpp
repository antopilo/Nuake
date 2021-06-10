#include "ScriptingEngine.h"
#include "WrenScript.h"
#include "../Core/FileSystem.h"
#include <src/Scripting/Modules/ScriptModule.h>
#include <src/Scripting/Modules/EngineModule.h>
#include <src/Scripting/Modules/SceneModule.h>
WrenVM* ScriptingEngine::m_WrenVM;


std::map<std::string, Ref<WrenScript>> ScriptingEngine::m_Scripts;

std::map<std::string, Ref<ScriptModule>> ScriptingEngine::Modules;

void errorFn(WrenVM* vm, WrenErrorType errorType,
    const char* module, const int line,
    const char* msg)
{
    switch (errorType)
    {
    case WREN_ERROR_COMPILE:
    {
        printf("[%s line %d] [Error] %s\n", module, line, msg);
    } break;
    case WREN_ERROR_STACK_TRACE:
    {
        printf("[%s line %d] in %s\n", module, line, msg);
    } break;
    case WREN_ERROR_RUNTIME:
    {
        printf("[Runtime Error] %s\n", msg);
    } break;
    }
}


void writeFn(WrenVM* vm, const char* text) {
	printf("%s", text);
}


WrenLoadModuleResult myLoadModule(WrenVM* vm, const char* name) {
    WrenLoadModuleResult result = { 0 };
    std::string str = FileSystem::ReadFile("resources/" + std::string(name) + ".wren", true);
    char* c = strcpy(new char[str.length() + 1], str.c_str());
    result.source = c;
    return result;
}

Ref<WrenScript> ScriptingEngine::RegisterScript(const std::string& path, const std::string& mod)
{
    std::string query = "import \"" + path + "\" for " + mod;
    wrenInterpret(m_WrenVM, "main", query.c_str());
    return CreateRef<WrenScript>(path, mod);
}

void ScriptingEngine::RegisterModule(Ref<ScriptModule> scriptModule)
{
    Modules[scriptModule->GetModuleName()] = scriptModule;
    scriptModule->RegisterModule(m_WrenVM);
}


void ScriptingEngine::InitScript(Ref<WrenScript> script)
{
    script->CallInit();
}

void ScriptingEngine::UpdateScript(Ref<WrenScript> script, Timestep timestep)
{
    script->CallUpdate(timestep);
}

void ScriptingEngine::ExitScript(Ref<WrenScript> script)
{
    script->CallExit();
}

void ScriptingEngine::Init()
{
    m_Scripts = std::map<std::string, Ref<WrenScript>>();
	WrenConfiguration config;
	wrenInitConfiguration(&config);

    config.loadModuleFn = &myLoadModule;
    config.errorFn = &errorFn;
	config.writeFn = &writeFn;
    config.bindForeignMethodFn = &bindForeignMethod;
	m_WrenVM = wrenNewVM(&config);

    Ref<ScriptAPI::EngineModule> engineModule = CreateRef<ScriptAPI::EngineModule>();
    RegisterModule(engineModule);
    Ref<ScriptAPI::SceneModule> sceneModule = CreateRef<ScriptAPI::SceneModule>();
    RegisterModule(sceneModule);
}


void ScriptingEngine::Close()
{
	wrenFreeVM(m_WrenVM);
}

WrenForeignMethodFn ScriptingEngine::bindForeignMethod(WrenVM* vm, const char* module, const char* className, bool isStatic, const char* signature)
{
    if (Modules.find(className) != Modules.end())
    {
        Ref<ScriptModule> mod = Modules[className];
        void* ptr = mod->GetMethodPointer(signature);
        return (WrenForeignMethodFn)ptr;
    }
}

WrenVM* ScriptingEngine::GetWrenVM()
{
    return m_WrenVM;
}

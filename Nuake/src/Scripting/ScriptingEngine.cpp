#include "ScriptingEngine.h"
#include "WrenScript.h"
#include "src/Core/FileSystem.h"

#include "src/Scripting/Modules/ScriptModule.h"
#include "src/Scripting/Modules/EngineModule.h"
#include "src/Scripting/Modules/SceneModule.h"
#include "src/Scripting/Modules/MathModule.h"
#include "src/Scripting/Modules/InputModule.h"
#include "src/Scripting/Modules/PhysicsModule.h"

namespace Nuake {
    WrenVM* ScriptingEngine::m_WrenVM;

    std::map<std::string, Ref<WrenScript>> ScriptingEngine::m_Scripts;
    std::map<std::string, Ref<ScriptModule>> ScriptingEngine::Modules;
    std::vector<std::string> ScriptingEngine::m_LoadedScripts;

    void errorFn(WrenVM* vm, WrenErrorType errorType,
        const char* module, const int line,
        const char* msg)
    {
        Logger::Log("YO");
        switch (errorType)
        {
        case WREN_ERROR_COMPILE:
        {
            std::string t = std::string(module) + " line " + std::to_string(line) + ": " + msg;
            Logger::Log(t, CRITICAL);
            Engine::ExitPlayMode();
        } break;
        case WREN_ERROR_STACK_TRACE:
        {
            std::string t = "Stack trace: " + std::string(module) + " line " + std::to_string(line) + ": " + msg;
            Logger::Log(t, CRITICAL);
        } break;
        case WREN_ERROR_RUNTIME:
        {
            std::string t = "Script Runtime Error: " + std::string(msg);
            Logger::Log(t, WARNING);
        } break;
        default:
        {
            std::string t = "Script Runtime Error: " + std::string(msg);
            Logger::Log(t, CRITICAL);
        }
            break;
        }
    }

    void writeFn(WrenVM* vm, const char* text)
    {
        printf("%s", text);
    }

    bool hasEnding(std::string const& fullString, std::string const& ending)
    {
        if (fullString.length() >= ending.length())
        {
            return (0 == fullString.compare(fullString.length() - ending.length(), ending.length(), ending));
        }
        else
        {
            return false;
        }
    }

    const std::string NuakeModulePrefix = "Nuake:";
    WrenLoadModuleResult myLoadModule(WrenVM* vm, const char* name)
    {
        WrenLoadModuleResult result = { 0 };

        std::string sname = std::string(name);
        std::string path = "resources/" + std::string(name);

        bool isAbsolute = false;
        if (sname.rfind(NuakeModulePrefix, 0) == 0)
        {
            size_t prefixPosition = sname.find(NuakeModulePrefix);
            sname.erase(prefixPosition, NuakeModulePrefix.length());
            path = "resources/Scripts/" + std::string(sname);
        }
        else
        {
            path = FileSystem::Root + name;
            isAbsolute = true;
        }

        if (!hasEnding(path, ".wren"))
            path += ".wren";

        std::string str = FileSystem::ReadFile(path, true);
        char* c = strcpy(new char[str.length() + 1], str.c_str());

        result.source = c;
        return result;
    }

    //Ref<WrenScript> ScriptingEngine::RegisterScript(const std::string& path, const std::string& mod)
    //{
    //    //// Check if scripts has already been loaded.
    //    //// You can't import the same module twice, otherwise, compile error.
    //    //if (m_Scripts.find(path) == m_Scripts.end())
    //    //{
    //    //    std::string query = "import \"" + path + "\" for " + mod;
    //    //    wrenInterpret(m_WrenVM, "main", query.c_str());
    //    //}
    //    //
    //    //return CreateRef<WrenScript>(path, mod);
    //}

    // Useful to check if a script has been imported, importing a script
    // twice gives a compile error.
    bool ScriptingEngine::IsScriptImported(const std::string& path)
    {
        for (auto& script : m_LoadedScripts)
        {
            if (script == path)
                return true;
        }
        return false;
    }

    void ScriptingEngine::ImportScript(const std::string& path)
    {
        if (IsScriptImported(path))
            return;
        m_LoadedScripts.push_back(path);
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
        Logger::Log("Initializing Scripting Engine");
        m_Scripts = std::map<std::string, Ref<WrenScript>>();
        m_LoadedScripts.clear();

        WrenConfiguration config;
        wrenInitConfiguration(&config);

        config.loadModuleFn = &myLoadModule;
        config.errorFn = &errorFn;
        config.writeFn = &writeFn;
        config.bindForeignMethodFn = &bindForeignMethod;
        m_WrenVM = wrenNewVM(&config);

        Logger::Log("Registing Scripting Modules");
        Ref<ScriptAPI::EngineModule> engineModule = CreateRef<ScriptAPI::EngineModule>();
        RegisterModule(engineModule);
        Ref<ScriptAPI::SceneModule> sceneModule = CreateRef<ScriptAPI::SceneModule>();
        RegisterModule(sceneModule);
        Ref<ScriptAPI::MathModule> mathModule = CreateRef<ScriptAPI::MathModule>();
        RegisterModule(mathModule);
        Ref<ScriptAPI::InputModule> inputModule = CreateRef<ScriptAPI::InputModule>();
        RegisterModule(inputModule);
        Ref<ScriptAPI::PhysicsModule> physicsModule = CreateRef<ScriptAPI::PhysicsModule>();
        RegisterModule(physicsModule);
        Logger::Log("Scripting Modules Registered");
        Logger::Log("Scripting Engine initialized successfully");
    }

    void ScriptingEngine::Close()
    {
        wrenFreeVM(m_WrenVM);
    }

    WrenForeignMethodFn ScriptingEngine::bindForeignMethod(WrenVM* vm, const char* modul, const char* className, bool isStatic, const char* signature)
    {
        if (Modules.find(className) != Modules.end())
        {
            Ref<ScriptModule> mod = Modules[className];
            void* ptr = mod->GetMethodPointer(signature);
            return (WrenForeignMethodFn)ptr;
        }

        return nullptr;
    }

    WrenVM* ScriptingEngine::GetWrenVM()
    {
        return m_WrenVM;
    }
}


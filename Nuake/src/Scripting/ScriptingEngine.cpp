
#include "ScriptingEngine.h"
#include "WrenScript.h"
#include "src/Core/FileSystem.h"

#include "src/Scripting/Modules/ScriptModule.h"
#include "src/Scripting/Modules/EngineModule.h"
#include "src/Scripting/Modules/SceneModule.h"
#include "src/Scripting/Modules/MathModule.h"
#include "src/Scripting/Modules/InputModule.h"
#include "src/Scripting/Modules/PhysicsModule.h"

#include "src/Resource/StaticResources.h"

namespace Nuake {
    WrenVM* ScriptingEngine::m_WrenVM;

    std::map<std::string, Ref<WrenScript>> ScriptingEngine::m_Scripts;
    std::map<std::string, Ref<ScriptModule>> ScriptingEngine::Modules;
    std::vector<std::string> ScriptingEngine::m_LoadedScripts;

    void errorFn(WrenVM* vm, WrenErrorType errorType,
        const char* moduleName, const int line,
        const char* msg)
    {
        switch (errorType)
        {
        case WREN_ERROR_COMPILE:
        {
            std::string t = std::string(moduleName) + " line " + std::to_string(line) + ": " + msg;
            Logger::Log(t, "script", CRITICAL);
            Engine::ExitPlayMode();
        } break;
        case WREN_ERROR_STACK_TRACE:
        {
            std::string t = "Stack trace: " + std::string(moduleName) + " line " + std::to_string(line) + ": " + msg;
            Logger::Log(t, "script", CRITICAL);
        } break;
        case WREN_ERROR_RUNTIME:
        {
            std::string t = "Script Runtime Error: " + std::string(msg);
            Logger::Log(t, "script", WARNING);
        } break;
        default:
        {
            std::string t = "Script Runtime Error: " + std::string(msg);
            Logger::Log(t, "script", CRITICAL);
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

#define GET_STATIC_RESOURCE_SCRIPT_SRC(name) std::string(reinterpret_cast<const char*>(name), reinterpret_cast<const char*>(name) + name##_len)

    std::map<std::string, std::string> _ModulesSRC =
    {
        { "Audio", GET_STATIC_RESOURCE_SCRIPT_SRC(StaticResources::Resources_Scripts_Audio_wren) },
        { "Engine", GET_STATIC_RESOURCE_SCRIPT_SRC(StaticResources::Resources_Scripts_Engine_wren) },
        { "Input", GET_STATIC_RESOURCE_SCRIPT_SRC(StaticResources::Resources_Scripts_Input_wren) },
        { "Math", GET_STATIC_RESOURCE_SCRIPT_SRC(StaticResources::Resources_Scripts_Math_wren) },
        { "Physics", GET_STATIC_RESOURCE_SCRIPT_SRC(StaticResources::Resources_Scripts_Physics_wren) },
        { "Scene", GET_STATIC_RESOURCE_SCRIPT_SRC(StaticResources::Resources_Scripts_Scene_wren) },
        { "ScriptableEntity", GET_STATIC_RESOURCE_SCRIPT_SRC(StaticResources::Resources_Scripts_ScriptableEntity_wren) },
    };

    void onCompleteCB(WrenVM* vm, const char* name, WrenLoadModuleResult result)
    {
        delete result.source;
    }

    const std::string NuakeModulePrefix = "Nuake:";
    WrenLoadModuleResult myLoadModule(WrenVM* vm, const char* name)
    {
        WrenLoadModuleResult result = { 0 };

        result.onComplete = &onCompleteCB; // This will take care of the clean up of the new char*
        std::string sname = std::string(name);
        std::string path = "resources/" + std::string(name);

        std::string fileContent = "";
        bool isAbsolute = false;
        if (sname.rfind(NuakeModulePrefix, 0) == 0)
        {
            size_t prefixPosition = sname.find(NuakeModulePrefix);
            sname.erase(prefixPosition, NuakeModulePrefix.length());
            path = "Resources/Scripts/"+std::string(sname);
            
            if (_ModulesSRC.find(sname) != _ModulesSRC.end())
            {
                fileContent = _ModulesSRC[sname];
            }
            else
            {
                Logger::Log("Internal script module not found: " + sname + ". \n Did you forget to register it?", "scripting engine", CRITICAL);
            }
        }
        else
        {
            path = FileSystem::Root + name;
            isAbsolute = true;
        }

        if (!hasEnding(path, ".wren"))
            path += ".wren";

        if (isAbsolute)
        {
            fileContent = FileSystem::ReadFile(path, true);
        }

        // We have to use c style char array and the callback takes care of cleaning up.
        char* sourceC = new char[fileContent.length() + 1];
        char* c = strcpy(sourceC, fileContent.c_str()); // copy into c array

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
        RegisterModule(CreateRef<ScriptAPI::EngineModule>());
        RegisterModule(CreateRef<ScriptAPI::SceneModule>());
        RegisterModule(CreateRef<ScriptAPI::MathModule>());
        RegisterModule(CreateRef<ScriptAPI::InputModule>());
        RegisterModule(CreateRef<ScriptAPI::PhysicsModule>());
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


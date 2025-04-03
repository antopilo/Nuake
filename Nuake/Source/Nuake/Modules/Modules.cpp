// Auto-generated modules management file
#include "Modules.h"

#include "AssimpModule/AssimpModule.h"
#include "AudioModule/AudioModule.h"
#include "ExampleModule/ExampleModule.h"

#include "Nuake/Core/Logger.h"
#include "Nuake/Modules/ModuleDB.h"

void Nuake::Modules::StartupModules()
{
    Logger::Log("Starting AssimpModule", "modules");
    AssimpModule_Startup();
    Logger::Log("Starting AudioModule", "modules");
    AudioModule_Startup();
    Logger::Log("Starting ExampleModule", "modules");
    ExampleModule_Startup();
}

void Nuake::Modules::FixedUpdate(float ts)
{
	auto& moduleDB = ModuleDB::Get();
    for (auto& moduleName : moduleDB.GetModules())
    {
		auto& module = moduleDB.GetBaseImpl(moduleName);
		module.OnFixedUpdate.Broadcast(ts);
    }
}

void Nuake::Modules::Update(float ts)
{
    auto& moduleDB = ModuleDB::Get();
    for (auto& moduleName : moduleDB.GetModules())
    {
        auto& module = moduleDB.GetBaseImpl(moduleName);
        module.OnUpdate.Broadcast(ts);
    }
}

void Nuake::Modules::ShutdownModules()
{
    Logger::Log("Shutting down AssimpModule", "modules");
    AssimpModule_Shutdown();
    Logger::Log("Shutting down AudioModule", "modules");
    AudioModule_Shutdown();
    Logger::Log("Shutting down ExampleModule", "modules");
    ExampleModule_Shutdown();
}

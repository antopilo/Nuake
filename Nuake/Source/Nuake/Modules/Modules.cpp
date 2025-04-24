// Auto-generated modules management file
#include "Modules.h"

#include "AssimpModule/AssimpModule.h"
#include "AudioModule/AudioModule.h"
#include "ExampleModule/ExampleModule.h"
#include "QuakeModule/QuakeModule.h"

#include "Nuake/Core/Logger.h"

void Nuake::Modules::StartupModules()
{
    Logger::Log("Starting AssimpModule", "modules");
    AssimpModule_Startup();
    Logger::Log("Starting AudioModule", "modules");
    AudioModule_Startup();
    Logger::Log("Starting ExampleModule", "modules");
    ExampleModule_Startup();
    Logger::Log("Starting QuakeModule", "modules");
    QuakeModule_Startup();
}

void Nuake::Modules::ShutdownModules()
{
    Logger::Log("Shutting down AssimpModule", "modules");
    AssimpModule_Shutdown();
    Logger::Log("Shutting down AudioModule", "modules");
    AudioModule_Shutdown();
    Logger::Log("Shutting down ExampleModule", "modules");
    ExampleModule_Shutdown();
    Logger::Log("Shutting down QuakeModule", "modules");
    QuakeModule_Shutdown();
}

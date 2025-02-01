// Auto-generated modules management file
#include "Modules.h"

#include "ExampleModule/ExampleModule.h"

#include "Nuake/Core/Logger.h"

void Nuake::Modules::StartupModules()
{
    Logger::Log("Starting ExampleModule", "modules");
    ExampleModule_Startup();
}

void Nuake::Modules::ShutdownModules()
{
    Logger::Log("Shutting down ExampleModule", "modules");
    ExampleModule_Shutdown();
}

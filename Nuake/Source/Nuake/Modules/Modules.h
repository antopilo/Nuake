#pragma once

#include <map>
#include <string>

namespace Nuake
{
    class Modules
    {
    private:
        std::map<std::string, void*> modules;

    public:
        static void StartupModules();
        static void ShutdownModules();
    };
}
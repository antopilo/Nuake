#pragma once

#include "EngineSubsystem.h"
#include "Coral/ManagedObject.hpp"

/**
 * Essentially just a wrapper for C# subsystems
 */
namespace Nuake
{
    class EngineSubsystemScript : public EngineSubsystem
    {
    public:
        EngineSubsystemScript(const Coral::ManagedObject& object);

        Coral::ManagedObject& GetManagedObjectInstance();

    private:
        Coral::ManagedObject cSharpObjectInstance;
    };
}

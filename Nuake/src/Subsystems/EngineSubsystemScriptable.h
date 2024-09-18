#pragma once

#include "EngineSubsystem.h"

#include <Coral/ManagedObject.hpp>

/**
 * Essentially just a wrapper for C# subsystems
 */
namespace Nuake
{
    class EngineSubsystemScriptable : public EngineSubsystem
    {
    public:
        EngineSubsystemScriptable(const Coral::ManagedObject& object);

        Coral::ManagedObject& GetManagedObjectInstance();

        virtual void Initialize() override;
        virtual void Tick(float deltaTime) override;

    private:
        Coral::ManagedObject cSharpObjectInstance;
    };
}

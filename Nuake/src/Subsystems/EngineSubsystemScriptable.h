#pragma once

#include "EngineSubsystem.h"
#include "src/Core/MulticastDelegate.h"

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
        virtual ~EngineSubsystemScriptable();

        Coral::ManagedObject& GetManagedObjectInstance();

        virtual void Initialize() override;
        virtual void Tick(float deltaTime) override;

        virtual void OnScenePreInitialize(Ref<Scene> scene) override;
        virtual void OnScenePostInitialize(Ref<Scene> scene) override;
        virtual void OnScenePreDestroy(Ref<Scene> scene) override;

    protected:
        void OnScriptEngineUninitialize();

        DelegateHandle scriptEngineUninitializeDelegateHandle;

        Coral::ManagedObject cSharpObjectInstance;
    };
}

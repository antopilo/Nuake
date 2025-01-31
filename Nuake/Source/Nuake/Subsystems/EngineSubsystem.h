﻿#pragma once

#include "Nuake/Core/Core.h"

/**
 * Specific type of subsystem that runs within the context of the engine, being created at the start of the
 * engine's lifetime and destroyed just before the engine shuts down.
 */
namespace Nuake
{
    class Scene;

    class EngineSubsystem
    {
    public:
        void SetCanTick(bool canTick);
        bool CanEverTick() const;

        virtual void Initialize() {}
        virtual void Tick(float deltaTime) {}

        virtual void OnScenePreInitialize(Ref<Scene> scene) {}
        virtual void OnScenePostInitialize(Ref<Scene> scene) {}
        virtual void OnScenePreDestroy(Ref<Scene> scene) {}

    protected:
        void OnScriptEngineUninitialize();
        
        bool canEverTick = false;
    };
}

#pragma once

/**
 * Specific type of subsystem that runs within the context of the engine, being created at the start of the
 * engine's lifetime and destroyed just before the engine shuts down.
 */
namespace Nuake
{
    class EngineSubsystem
    {
    public:
        void SetCanTick(bool canTick);
        bool CanEverTick() const;

        virtual void Initialize() {}
        virtual void Tick(float deltaTime) {}

    private:
        bool canEverTick = true;
    };
}

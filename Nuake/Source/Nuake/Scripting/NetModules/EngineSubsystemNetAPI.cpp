#include "EngineSubsystemNetAPI.h"

#include "Engine.h"
#include "Nuake/Subsystems/EngineSubsystemScriptable.h"

namespace Nuake
{
    void SetCanTick(int subsystemId, bool tick)
    {
        auto subsystem = Engine::GetScriptedSubsystem(subsystemId);
        if (subsystem == nullptr)
        {
            Logger::Log("Subsystem isn't a valid scripted subsystem", "EngineSubsystemNetAPI", WARNING);
            return;
        }

        subsystem->SetCanTick(tick);
    }

    bool GetCanTick(int subsystemId)
    {
        auto subsystem = Engine::GetScriptedSubsystem(subsystemId);
        if (subsystem == nullptr)
        {
            Logger::Log("Subsystem isn't a valid scripted subsystem", "EngineSubsystemNetAPI", WARNING);
            return false;
        }

        return subsystem->CanEverTick();
    }

    void EngineSubsystemNetAPI::RegisterMethods()
    {
        RegisterMethod("EngineSubsystem.SetCanTickIcall", &SetCanTick);
        RegisterMethod("EngineSubsystem.GetCanTickIcall", &GetCanTick);
    }
}

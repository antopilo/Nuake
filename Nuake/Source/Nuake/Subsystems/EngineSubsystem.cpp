#include "EngineSubsystem.h"

#include "Nuake/Scripting/ScriptingEngineNet.h"

namespace Nuake
{
    void EngineSubsystem::SetCanTick(bool canTick)
    {
        canEverTick = canTick;
    }

    bool EngineSubsystem::CanEverTick() const
    {
        return canEverTick;
    }
}



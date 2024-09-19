#include "EngineSubsystem.h"

#include "src/Scripting/ScriptingEngineNet.h"

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



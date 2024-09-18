#include "EngineSubsystem.h"

void Nuake::EngineSubsystem::SetCanTick(bool canTick)
{
    canEverTick = canTick;
}

bool Nuake::EngineSubsystem::CanEverTick() const
{
    return canEverTick;
}

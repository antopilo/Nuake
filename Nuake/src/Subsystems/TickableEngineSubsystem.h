#pragma once

#include "EngineSubsystem.h"

namespace Nuake
{
    class TickableEngineSubsystem : public EngineSubsystem
    {
    public:
        virtual void Tick() {}
    };
}


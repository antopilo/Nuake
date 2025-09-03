#pragma once

#include "NetAPIModule.h"

namespace Nuake
{
    class EngineSubsystemNetAPI : public Nuake::NetAPIModule
    {
    public:
        virtual const std::string GetModuleName() const override { return "EngineSubsystem"; }
        virtual void RegisterMethods() override;
    };
}


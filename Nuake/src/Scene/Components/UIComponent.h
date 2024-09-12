#pragma once

#include "Component.h"
#include "src/Core/Logger.h"

#include "src/Resource/UUID.h"
#include "src/Resource/Serializable.h"

namespace Nuake
{
    struct UIComponent : public Component
    {
        NUAKECOMPONENT(UIComponent, "UI")

    public:
        UUID UIResource = UUID(0);
        std::string CSharpUIController;
        std::string UIFilePath;
        bool IsWorldSpace;
        // TODO: Z-Ordering

        json Serialize()
        {
            BEGIN_SERIALIZE();
            SERIALIZE_VAL(UIFilePath);
            END_SERIALIZE();
        }

        bool Deserialize(const json& j)
        {
            DESERIALIZE_VAL(UIFilePath);
            return true;
        }
    };
}

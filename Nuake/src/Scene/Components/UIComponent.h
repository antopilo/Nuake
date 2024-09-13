#pragma once

#include "Component.h"
#include "FieldTypes.h"
#include "src/Core/Logger.h"
#include "src/FileSystem/File.h"
#include "src/FileSystem/FileSystem.h"

#include "src/Resource/UUID.h"
#include "src/Resource/Serializable.h"

namespace Nuake
{
    struct UIComponent : public Component
    {
        NUAKECOMPONENT(UIComponent, "UI")
        
        static void InitializeComponentClass()
        {
            BindComponentField<&UIComponent::UIResource>("UIResource", "UIResource");
                SetFlags(ComponentFieldTrait::Internal, ComponentFieldTrait::Transient);

            BindComponentField<&UIComponent::UIFilePath>("UIFilePath", "File Path");
            BindComponentField<&UIComponent::IsWorldSpace>("IsWorldspace", "Is Worldspace");
        }

    public:
        UUID UIResource = UUID(0);
        ResourceFile UIFilePath;
        bool IsWorldSpace;
        // TODO: Z-Ordering

        json Serialize()
        {
            BEGIN_SERIALIZE();
            SERIALIZE_RES_FILE(UIFilePath);
            END_SERIALIZE();
        }

        bool Deserialize(const json& j)
        {
            DESERIALIZE_RES_FILE(UIFilePath);
            return true;
        }
    };
}

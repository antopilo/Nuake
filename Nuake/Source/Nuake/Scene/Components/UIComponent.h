#pragma once

#include "Component.h"
#include "Nuake/Core/Maths.h"
#include "FieldTypes.h"
#include "Nuake/Core/Logger.h"
#include "Nuake/FileSystem/File.h"
#include "Nuake/FileSystem/FileSystem.h"

#include "Nuake/Resource/UUID.h"
#include "Nuake/Resource/Serializable.h"

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
            BindComponentProperty<&UIComponent::SetResolution, &UIComponent::GetResolution>("Resolution", "Resolution");
        }

    public:
        UUID UIResource = UUID(0);
        ResourceFile UIFilePath;
        bool IsWorldSpace;
        Vector3 Resolution = Vector3(1920, 1080, 0);
        // TODO: Z-Ordering

        void SetUIFile(ResourceFile file);
        ResourceFile GetUIFile();

        void SetResolution(const Vector3& newSize)
        {
            Resolution = newSize;
        }

        Vector3 GetResolution()
        {
            return Resolution;
        }

        json Serialize()
        {
            BEGIN_SERIALIZE();
            SERIALIZE_RES_FILE(UIFilePath);
            SERIALIZE_VAL(IsWorldSpace);
            SERIALIZE_VEC3(Resolution)
            END_SERIALIZE();
        }

        bool Deserialize(const json& j)
        {
            DESERIALIZE_RES_FILE(UIFilePath);
            DESERIALIZE_VAL(IsWorldSpace);
            if (j.contains("Resolution"))
            {
                DESERIALIZE_VEC3(j["Resolution"], Resolution);
            }
            return true;
        }
    };
}

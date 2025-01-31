#pragma once

#include "Component.h"
#include "Nuake/Core/Core.h"
#include "Nuake/Resource/Serializable.h"
#include "Nuake/Resource/UUID.h"
namespace Nuake
{
    class SkyComponent : public Component
    {
        NUAKECOMPONENT(SkyComponent, "Sky Component")

        static void InitializeComponentClass()
        {
            BindComponentField<&SkyComponent::SkyResource>("SkyResource", "Sky Resource");
                SetFlags(ComponentFieldTrait::Internal, ComponentFieldTrait::Transient);
            BindComponentField<&SkyComponent::SkyResourceFilePath>("SkyResourceFilePath", "File Path");
            ResourceFileRestriction("_SkyFile");

           //BindComponentField<&BoxColliderComponent::IsTrigger>("IsTrigger", "Is Trigger");
           //BindComponentProperty<&BoxColliderComponent::SetSize, &BoxColliderComponent::GetSize>("Size", "Size");
        }

    public:
        SkyComponent() = default;
        ~SkyComponent() = default;

        UUID SkyResource = UUID(0);
        ResourceFile SkyResourceFilePath;

        json Serialize();
        bool Deserialize(const json& j);
    };
}
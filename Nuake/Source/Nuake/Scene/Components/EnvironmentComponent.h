#pragma once

#include "Component.h"

#include "Nuake/Core/Core.h"
#include "Nuake/Resource/Serializable.h"
#include "Nuake/Resource/UUID.h"

namespace Nuake
{
    class EnvironmentComponent : public Component
    {
        NUAKECOMPONENT(EnvironmentComponent, "Environment")

        static void InitializeComponentClass()
        {
            BindComponentField<&EnvironmentComponent::EnvResource>("EnvResource", "Environment Resource");
                SetFlags(ComponentFieldTrait::Internal, ComponentFieldTrait::Transient);
            BindComponentField<&EnvironmentComponent::EnvResourceFilePath>("EnvResourceFilePath", "File Path");
            ResourceFileRestriction("_EnvFile");
        }

    public:
        UUID EnvResource = UUID(0);
        ResourceFile EnvResourceFilePath;

        json Serialize();
        bool Deserialize(const json& j);
    };
}

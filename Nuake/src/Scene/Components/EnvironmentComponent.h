#pragma once

#include "Component.h"

#include "src/Core/Core.h"
#include "src/Resource/Serializable.h"
#include "src/Resource/UUID.h"

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

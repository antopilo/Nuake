#pragma once

#include "Component.h"

#include "src/Core/Core.h"
#include "src/Resource/Serializable.h"

namespace Nuake
{
	class BoneComponent : public Component
	{
        NUAKECOMPONENT(BoneComponent, "Bone")

	public:
		BoneComponent() = default;
		~BoneComponent() = default;

		std::string Name;

        json Serialize()
        {
            BEGIN_SERIALIZE();
            SERIALIZE_VAL(Name);
            END_SERIALIZE();
        }

        bool Deserialize(const json& j)
        {
            Name = j["Name"];
           
            return true;
        }
	};
}
#include "CapsuleColliderComponent.h"
#include "Nuake/Resource/Serializable.h"

namespace Nuake
{
	json CapsuleColliderComponent::Serialize()
	{
		BEGIN_SERIALIZE()

		j["IsTrigger"] = IsTrigger;
		SERIALIZE_VAL(Radius)
		SERIALIZE_VAL(Height)
		END_SERIALIZE()
	}

	bool CapsuleColliderComponent::Deserialize(const json& j)
	{
		this->IsTrigger = j["IsTrigger"];
		this->Radius = j["Radius"];
		this->Height = j["Height"];
		return true;
	}
}
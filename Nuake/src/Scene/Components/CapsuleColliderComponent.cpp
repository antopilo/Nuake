#include "CapsuleColliderComponent.h"
#include "src/Resource/Serializable.h"

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

	bool CapsuleColliderComponent::Deserialize(const std::string& str)
	{
		BEGIN_DESERIALIZE()
		this->IsTrigger = j["IsTrigger"];
		this->Radius = j["Radius"];
		this->Height = j["Height"];
		return true;
	}
}
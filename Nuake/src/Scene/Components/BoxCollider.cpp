#include "BoxCollider.h"
#include "src/Resource/Serializable.h"

namespace Nuake
{
	json BoxColliderComponent::Serialize()
	{
		BEGIN_SERIALIZE();

		j["IsTrigger"] = IsTrigger;
		SERIALIZE_VEC3(Size);
		END_SERIALIZE();
	}

	bool BoxColliderComponent::Deserialize(const json& j)
	{
		this->IsTrigger = j["IsTrigger"];
		this->Size = Vector3(j["Size"]["x"], j["Size"]["y"], j["Size"]["z"]);
		return true;
	}
}
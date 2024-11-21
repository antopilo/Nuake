#pragma once

#include "Component.h"

#include "src/Physics/PhysicsShapes.h"
#include "src/Core/Core.h"

namespace Nuake
{
	class MeshColliderComponent : public Component
	{
		NUAKECOMPONENT(MeshColliderComponent, "Mesh Collider")

	public:
		uint32_t SubMesh = 0;
		bool IsTrigger;
		Ref<Physics::MeshShape> Shape;

		json Serialize()
		{
			BEGIN_SERIALIZE();
			j["IsTrigger"] = IsTrigger;
			SERIALIZE_VAL(SubMesh);
			END_SERIALIZE();
		}

		bool Deserialize(const json& j)
		{
			this->IsTrigger = j["IsTrigger"];
			this->SubMesh = j["SubMesh"];
			return true;
		}
	};
}

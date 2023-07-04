#pragma once
#include "src/Core/Physics/PhysicsShapes.h"
#include "src/Core/Core.h"

namespace Nuake {
	class MeshColliderComponent
	{
	public:
		int SubMesh = 0;
		bool IsTrigger;
		Ref<Physics::MeshShape> Shape;

		json Serialize()
		{
			BEGIN_SERIALIZE();
			j["IsTrigger"] = IsTrigger;
			SERIALIZE_VAL(SubMesh);
			END_SERIALIZE();
		}

		bool Deserialize(const std::string& str)
		{
			BEGIN_DESERIALIZE();
			this->IsTrigger = j["IsTrigger"];
			this->SubMesh = j["SubMesh"];
			return true;
		}
	};
}

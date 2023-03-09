#pragma once
#include "src/Core/Physics/PhysicsShapes.h"
#include "src/Core/Core.h"

namespace Nuake {
	class SphereColliderComponent
	{
	public:
		Ref<Physics::PhysicShape> Sphere;

		float Radius = 0.5f;
		bool IsTrigger = false;

		json Serialize()
		{
			BEGIN_SERIALIZE();
			j["Radius"] = Radius;
			j["IsTrigger"] = IsTrigger;
			END_SERIALIZE();
		}

		bool Deserialize(const std::string& str)
		{
			BEGIN_DESERIALIZE();
			this->Radius = j["Radius"];
			this->IsTrigger = j["IsTrigger"];
			return true;
		}
	};
}

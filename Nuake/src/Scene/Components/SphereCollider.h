#pragma once

#include "src/Core/Object/Object.h"
#include "src/Physics/PhysicsShapes.h"
#include "src/Core/Core.h"

namespace Nuake {
	class SphereColliderComponent
	{
		NUAKECOMPONENT(SphereColliderComponent, "Sphere Component")

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

		bool Deserialize(const json& j)
		{
			this->Radius = j["Radius"];
			this->IsTrigger = j["IsTrigger"];
			return true;
		}
	};
}

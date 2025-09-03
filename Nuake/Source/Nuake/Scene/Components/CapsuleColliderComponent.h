#pragma once

#include "Component.h"

#include "Nuake/Physics/PhysicsShapes.h"
#include "Nuake/Core/Core.h"

namespace Nuake
{
	class CapsuleColliderComponent : public Component
	{
		NUAKECOMPONENT(CapsuleColliderComponent, "Capsule Collider")

	public:
		Ref<Physics::Capsule> Capsule;

		float Radius = 0.5f;
		float Height = 1.0f;

		bool IsTrigger = false;

		json Serialize();
		bool Deserialize(const json& str);
	};
}

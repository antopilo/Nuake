#pragma once
#include "src/Core/Physics/PhysicsShapes.h"
#include "src/Core/Core.h"

namespace Nuake
{
	class CapsuleColliderComponent
	{
	public:
		Ref<Physics::Capsule> Capsule;

		float Radius = 0.5f;
		float Height = 1.0f;

		bool IsTrigger = false;

		json Serialize();
		bool Deserialize(const json& str);
	};
}

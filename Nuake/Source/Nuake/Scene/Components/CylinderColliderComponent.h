#pragma once

#include "Component.h"

#include "Nuake/Physics/PhysicsShapes.h"
#include "Nuake/Core/Core.h"

namespace Nuake
{
	class CylinderColliderComponent : public Component
	{
		NUAKECOMPONENT(CylinderColliderComponent, "Cylinder Collider")

	public:
		Ref<Physics::Cylinder> Cylinder;

		float Radius = 0.5f;
		float Height = 1.0f;

		bool IsTrigger = false;

		json Serialize();
		bool Deserialize(const json& j);
	};
}

#pragma once

#include "src/Core/Object/Object.h"
#include "src/Physics/PhysicsShapes.h"
#include "src/Core/Core.h"

namespace Nuake
{
	class CylinderColliderComponent
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

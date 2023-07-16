#pragma once
#include "src/Core/Physics/PhysicsShapes.h"
#include "src/Core/Core.h"

namespace Nuake
{
	class CylinderColliderComponent
	{
	public:
		Ref<Physics::Cylinder> Cylinder;

		float Radius = 0.5f;
		float Height = 1.0f;

		bool IsTrigger = false;

		json Serialize();
		bool Deserialize(const std::string& str);
	};
}

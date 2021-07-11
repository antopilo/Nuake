#pragma once
#include "src/Core/Physics/PhysicsShapes.h"
#include "src/Core/Core.h"

namespace Nuake {
	class SphereColliderComponent
	{
	public:
		Ref<Physics::PhysicShape> Sphere;
		float Radius = 0.5f;
		bool IsTrigger;
	};
}

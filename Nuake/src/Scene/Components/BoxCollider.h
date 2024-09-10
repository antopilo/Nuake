#pragma once

#include "src/Core/Object/Object.h"
#include "src/Physics/PhysicsShapes.h"
#include "src/Core/Core.h"

namespace Nuake {

	class BoxColliderComponent
	{
		NUAKECOMPONENT(BoxColliderComponent, "Box Collider")

	public:
		Ref<Physics::PhysicShape> Box;
		Vector3 Size = Vector3(0.5f, 0.5f, 0.5f);
		bool IsTrigger = false;

		json Serialize();
		bool Deserialize(const json& j);
	};
}
 
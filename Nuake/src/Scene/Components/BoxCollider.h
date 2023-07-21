#pragma once
#include "src/Core/Physics/PhysicsShapes.h"
#include "src/Core/Core.h"

namespace Nuake {
	class BoxColliderComponent
	{
	public:
		Ref<Physics::PhysicShape> Box;
		Vector3 Size = Vector3(0.5f, 0.5f, 0.5f);
		bool IsTrigger = false;

		json Serialize();
		bool Deserialize(const std::string& str);
	};
}
 
#pragma once
#include "src/Core/Physics/PhysicsShapes.h"
#include "src/Core/Core.h"
namespace Nuake {
	class BoxColliderComponent
	{
	public:
		Ref<Physics::PhysicShape> Box;
		glm::vec3 Size = glm::vec3(0.5f, 0.5f, 0.5f);
		bool IsTrigger = false;

		json Serialize();
		bool Deserialize(const std::string& str);
	};
}
 
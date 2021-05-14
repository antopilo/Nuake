#pragma once
#include "../Core/Physics/PhysicsShapes.h"
#include "../Core/Core.h"

class BoxColliderComponent
{
public:
	Ref<Physics::PhysicShape> Box;
	glm::vec3 Size = glm::vec3(0.5f, 0.5f, 0.5f);
	bool IsTrigger;
};
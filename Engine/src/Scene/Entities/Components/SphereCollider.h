#pragma once
#include "../Core/Physics/PhysicsShapes.h"
#include "../Core/Core.h"

class SphereColliderComponent
{
public:
	Ref<Physics::PhysicShape> Sphere;
	float Radius = 0.5f;
	bool IsTrigger;
};
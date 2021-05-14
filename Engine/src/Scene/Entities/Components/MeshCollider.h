#pragma once
#include "../Core/Physics/PhysicsShapes.h"
#include "../Core/Core.h"

class MeshColliderComponent
{
public:
	Ref<Physics::MeshShape> MeshShape;
	Ref<Mesh> Mesh;
	bool IsTrigger;
};
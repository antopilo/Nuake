#pragma once
#include "src/Core/Physics/PhysicsShapes.h"
#include "src/Core/Core.h"

namespace Nuake {
	class MeshColliderComponent
	{
	public:
		Ref<Physics::MeshShape> MeshShape;
		Ref<Mesh> Mesh;
		bool IsTrigger;
	};
}

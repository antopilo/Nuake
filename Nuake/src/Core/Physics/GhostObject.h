#pragma once
#include "src/Core/Core.h"
#include "src/Core/Maths.h"
#include "PhysicsShapes.h"

#include <vector>
#include <Engine.h>


class btGhostObject;

namespace Nuake
{
	class GhostObject {
	private:
		btGhostObject* m_BulletObject;
		std::vector<Entity> m_OverlappingEntities;

	public:
		GhostObject(Vector3 position, Ref<Physics::PhysicShape> shape);

		int OverlappingCount();
		void ClearOverlappingList();
		void ScanOverlap();

		void SetEntityID(Entity ent);

		std::vector<Entity> GetOverlappingEntities();

		// Internal use only.
		btGhostObject* GetBulletObject();
	};
}

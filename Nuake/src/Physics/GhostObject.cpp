#include "GhostObject.h"

#include "src/Physics/PhysicsManager.h"

namespace Nuake
{
	GhostObject::GhostObject(Vector3 position, Ref<Physics::PhysicShape> shape)
	{
		m_OverlappingEntities = std::vector<Entity>();
	}

	int GhostObject::OverlappingCount()
	{
		return 0;
	}

	void GhostObject::ClearOverlappingList()
	{
		m_OverlappingEntities.clear();
	}

	void GhostObject::SetEntityID(Entity ent)
	{
	}

	void GhostObject::ScanOverlap()
	{
		ClearOverlappingList();

		for (int i = 0; i < OverlappingCount(); i++)
		{
			int index =0;
			if (index == -1)
				continue;

			Entity handle = Engine::GetCurrentScene()->GetEntity(index);
			m_OverlappingEntities.push_back(handle);
		}
	}

	std::vector<Entity> GhostObject::GetOverlappingEntities()
	{
		return m_OverlappingEntities;
	}

}

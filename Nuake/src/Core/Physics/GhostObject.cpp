#include "GhostObject.h"
#include <dependencies/bullet3/src/BulletCollision/CollisionDispatch/btGhostObject.h>
#include <dependencies/bullet3/src/BulletCollision/BroadphaseCollision/btCollisionAlgorithm.h>
#include <dependencies/bullet3/src/BulletDynamics/Dynamics/btDiscreteDynamicsWorld.h>
#include <src/Core/Physics/PhysicsManager.h>

namespace Nuake
{
	GhostObject::GhostObject(Vector3 position, Ref<Physics::PhysicShape> shape)
	{
		m_OverlappingEntities = std::vector<Entity>();
		m_BulletObject = new btGhostObject();

		btTransform transform = btTransform();
		transform.setIdentity();
		transform.setOrigin(btVector3(position.x, position.y, position.z));

		m_BulletObject->setWorldTransform(transform);
		m_BulletObject->setCollisionShape(shape->GetBulletShape());
	}

	int GhostObject::OverlappingCount()
	{
		return m_BulletObject->getNumOverlappingObjects();
	}

	void GhostObject::ClearOverlappingList()
	{
		m_OverlappingEntities.clear();
	}

	void GhostObject::SetEntityID(Entity ent)
	{
		m_BulletObject->setUserIndex(ent.GetHandle());
	}

	void GhostObject::ScanOverlap()
	{
		ClearOverlappingList();

		for (int i = 0; i < OverlappingCount(); i++)
		{
			int index = m_BulletObject->getOverlappingObject(i)->getUserIndex();
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

	// Internal use only.
	btGhostObject* GhostObject::GetBulletObject()
	{
		return m_BulletObject;
	}
}

#include "GhostObject.h"
#include <dependencies/bullet3/src/BulletCollision/CollisionDispatch/btGhostObject.h>


GhostObject::GhostObject(Vector3 position, Ref<Physics::PhysicShape> shape) 
{
	m_OverlappingEntities = std::vector<Entity>();
	m_BulletObject = new btGhostObject();

	btTransform transform;
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
		Entity handle = Engine::GetCurrentScene()->GetEntity(m_BulletObject->getOverlappingObject(i)->getUserIndex());
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
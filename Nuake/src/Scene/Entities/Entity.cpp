

#include "Components/ParentComponent.h"
#include "Entity.h"
void Entity::AddChild(Entity ent)
{
	if ((int)m_EntityHandle != ent.GetHandle())
	{
		ent.GetComponent<ParentComponent>().HasParent = true;
		ent.GetComponent<ParentComponent>().Parent = *this;

		GetComponent<ParentComponent>().Children.push_back(ent);
	}
}

Entity::Entity(entt::entity handle, Scene* scene)
{
	m_EntityHandle = handle;
	m_Scene = scene;
}

Entity::Entity(const Entity& ent)
{
	this->m_EntityHandle = ent.m_EntityHandle;
	this->m_Scene = ent.m_Scene;
}


Entity::Entity(){}
#pragma once

#include "Entity.h"
#include "../Scene/Entities/Components/ParentComponent.h"
#include "../Scene/Entities/Components/NameComponent.h"

class ScriptableEntity
{
public:
	template<typename T>
	T& GetComponent()
	{
		return m_Entity.GetComponent<T>();
	}


	virtual void OnCreate() {}
	virtual void OnDestroy() {}
	virtual void OnUpdate(Timestep ts){}

	Entity m_Entity;
	friend class Scene;
};
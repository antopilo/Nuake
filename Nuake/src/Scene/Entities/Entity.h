#pragma once
#include <string>
#include <glm\ext\matrix_float4x4.hpp>
#include "../Scene.h"
#include "Components/BaseComponent.h"
class __declspec(dllexport) Entity
{
public:
	Entity(entt::entity handle, Scene* scene);
	Entity(const Entity& ent);
	Entity();


	void AddChild(Entity ent);

	int GetHandle() { return (int)m_EntityHandle; }
	template<typename T>
	bool HasComponent() {
		return m_Scene->m_Registry.has<T>(m_EntityHandle);
	}

	template<typename T>
	T& AddComponent() {
		T& component = m_Scene->m_Registry.emplace<T>(m_EntityHandle);
		return component;
	}
	
	template<typename T>
	void RemoveComponent() {
		m_Scene->m_Registry.remove<T>(m_EntityHandle);
	}

	template<typename T>
	T& GetComponent() {
		T& component = m_Scene->m_Registry.get<T>(m_EntityHandle);
		return component;
	}

	void Destroy() {
		m_Scene->m_Registry.destroy(m_EntityHandle);
	}

	bool operator==(const Entity& other) const
	{
		return m_EntityHandle == other.m_EntityHandle && m_Scene == other.m_Scene;
	}

	bool operator!=(const Entity& other) const
	{
		return !(*this == other);
	}
private:
	entt::entity m_EntityHandle;
	Scene* m_Scene;
};
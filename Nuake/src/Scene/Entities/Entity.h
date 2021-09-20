#pragma once
#include <string>
#include <glm/ext/matrix_float4x4.hpp>
#include "../Scene.h"
#include "../Components/BaseComponent.h"
#include "../Resource/Serializable.h"
#include "../Components/NameComponent.h"

namespace Nuake
{
	class Entity : public ISerializable
	{
	public:
		Entity(entt::entity handle, Scene* scene);
		Entity(const Entity& ent);
		Entity();

		void AddChild(Entity ent);

		int GetHandle() { return (int)m_EntityHandle; }
		int GetID() { return GetComponent<NameComponent>().ID; }

		template<typename T>
		bool HasComponent() {
			return m_Scene->m_Registry.all_of<T>(m_EntityHandle);
		}

		bool IsValid() {
			return m_Scene->m_Registry.valid((entt::entity)GetHandle());
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
			Logger::Log("Deleted eneity :" + std::to_string((int)m_EntityHandle));
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

		json Serialize() override;
		bool Deserialize(const std::string& str);

		Scene* GetScene() {
			return m_Scene;
		}
	private:
		entt::entity m_EntityHandle;
		Scene* m_Scene;
	};
}

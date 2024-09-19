#pragma once
#include <string>
#include <glm/ext/matrix_float4x4.hpp>

#include "src/Core/Logger.h"
#include "../Scene.h"
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

		int GetHandle() const { return (int)m_EntityHandle; }
		int GetID() const { return GetComponent<NameComponent>().ID; }

		template<typename T>
		bool HasComponent() const
		{
			return m_Scene->m_Registry.all_of<T>(m_EntityHandle);
		}

		bool EntityContainsItself(Entity a, Entity b);

		bool IsValid() const
		{
			return m_EntityHandle != (entt::entity)-1 && m_Scene->m_Registry.valid((entt::entity)GetHandle());
		}

		void AddComponent(entt::meta_type& enttMetaType)
		{
			if (!enttMetaType)
			{
				Logger::Log("Meta data empty/invalid", "Entity", WARNING);
				return;
			}

			entt::meta_any newComponent = enttMetaType.construct();
			if (!newComponent)
			{
				Logger::Log("Could not create a component from the meta type", "Entity", CRITICAL);
				return;
			}

			auto func = enttMetaType.func(HashedFnName::AddToEntity);
			// TODO: [WiggleWizard] Needs a lot more validation
			if (!func)
			{
				Logger::Log("No such function exists or is registered on component", "Entity", CRITICAL);
				return;
			}

			func.invoke(newComponent, m_EntityHandle, &m_Scene->m_Registry);
		}

		void RemoveComponent(entt::meta_type& enttMetaType)
		{
			if (!enttMetaType)
			{
				Logger::Log("Meta data empty/invalid", "Entity", WARNING);
				return;
			}
			
			entt::meta_any newComponent = enttMetaType.construct();
			if (!newComponent)
			{
				Logger::Log("Could not create a component from the meta type", "Entity", CRITICAL);
				return;
			}
			
			auto func = enttMetaType.func(HashedFnName::RemoveFromEntity);
			// TODO: [WiggleWizard] Needs a lot more validation
			if (!func)
			{
				Logger::Log("No such function exists or is registered on component", "Entity", CRITICAL);
				return;
			}

			func.invoke(newComponent, m_EntityHandle, &m_Scene->m_Registry);
		}

		template<typename T>
		T& AddComponent() 
		{
			T& component = m_Scene->m_Registry.emplace_or_replace<T>(m_EntityHandle);
			return component;
		}

		template<typename T>
		void RemoveComponent() 
		{
			m_Scene->m_Registry.remove<T>(m_EntityHandle);
		}

		template<typename T>
		T& GetComponent() 
		{
			T& component = m_Scene->m_Registry.get<T>(m_EntityHandle);
			return component;
		}

		template<typename T>
		T GetComponent() const
		{
			T component = m_Scene->m_Registry.get<T>(m_EntityHandle);
			return component;
		}

		void Destroy() 
		{
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
		bool Deserialize(const json& str);
		void PostDeserialize();

		Scene* GetScene() const
		{
			return m_Scene;
		}

	private:
		entt::entity m_EntityHandle;
		Scene* m_Scene;
	};
}

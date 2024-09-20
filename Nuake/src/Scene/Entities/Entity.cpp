#include "Entity.h"

#include "src/Scene/Components.h"


namespace Nuake
{
	void Entity::AddChild(Entity ent)
	{
		if ((int)m_EntityHandle != ent.GetHandle())
		{
			ent.GetComponent<ParentComponent>().HasParent = true;
			ent.GetComponent<ParentComponent>().Parent = *this;

			GetComponent<ParentComponent>().Children.push_back(ent);
		}
	}

	bool Entity::EntityContainsItself(Entity a, Entity b)
	{
		ParentComponent& targeParentComponent = b.GetComponent<ParentComponent>();
		if (!targeParentComponent.HasParent)
			return false;

		Entity currentParent = b.GetComponent<ParentComponent>().Parent;
		while (currentParent != a)
		{
			if (currentParent.GetComponent<ParentComponent>().HasParent)
				currentParent = currentParent.GetComponent<ParentComponent>().Parent;
			else
				return false;

			if (currentParent == a)
				return true;
		}
		return true;
	}

	json Entity::Serialize()
	{
		BEGIN_SERIALIZE();
		SERIALIZE_OBJECT_REF_LBL("NameComponent", GetComponent<NameComponent>())
		SERIALIZE_OBJECT_REF_LBL("ParentComponent", GetComponent<ParentComponent>())
		SERIALIZE_OBJECT_REF_LBL("TransformComponent", GetComponent<TransformComponent>())
		SERIALIZE_OBJECT_REF_LBL("VisibilityComponent", GetComponent<VisibilityComponent>())
		if (HasComponent<CameraComponent>())
			SERIALIZE_OBJECT_REF_LBL("CameraComponent", GetComponent<CameraComponent>())
		if (HasComponent<ParticleEmitterComponent>())
			SERIALIZE_OBJECT_REF_LBL("ParticleEmitterComponent", GetComponent<ParticleEmitterComponent>())
		if (HasComponent<SpriteComponent>())
			SERIALIZE_OBJECT_REF_LBL("SpriteComponent", GetComponent<SpriteComponent>())
		if (HasComponent<QuakeMapComponent>())
			SERIALIZE_OBJECT_REF_LBL("QuakeMapComponent", GetComponent<QuakeMapComponent>())
		if (HasComponent<LightComponent>())
			SERIALIZE_OBJECT_REF_LBL("LightComponent", GetComponent<LightComponent>())
		if (HasComponent<CharacterControllerComponent>())
			SERIALIZE_OBJECT_REF_LBL("CharacterControllerComponent", GetComponent<CharacterControllerComponent>())
		if (HasComponent<BoxColliderComponent>())
			SERIALIZE_OBJECT_REF_LBL("BoxColliderComponent", GetComponent<BoxColliderComponent>())
		if (HasComponent<CapsuleColliderComponent>())
			SERIALIZE_OBJECT_REF_LBL("CapsuleColliderComponent", GetComponent<CapsuleColliderComponent>())
		if (HasComponent<CylinderColliderComponent>())
			SERIALIZE_OBJECT_REF_LBL("CylinderColliderComponent", GetComponent<CylinderColliderComponent>())
		if (HasComponent<SphereColliderComponent>())
			SERIALIZE_OBJECT_REF_LBL("SphereColliderComponent", GetComponent<SphereColliderComponent>())
		if (HasComponent<MeshColliderComponent>())
			SERIALIZE_OBJECT_REF_LBL("MeshColliderComponent", GetComponent<MeshColliderComponent>())
		if (HasComponent<ModelComponent>())
			SERIALIZE_OBJECT_REF_LBL("ModelComponent", GetComponent<ModelComponent>())
		if (HasComponent<BSPBrushComponent>())
			SERIALIZE_OBJECT_REF_LBL("BSPBrushComponent", GetComponent<BSPBrushComponent>())
		if (HasComponent<QuakeMapComponent>())
			SERIALIZE_OBJECT_REF_LBL("QuakeMapComponent", GetComponent<QuakeMapComponent>())
		if (HasComponent<RigidBodyComponent>())
			SERIALIZE_OBJECT_REF_LBL("RigidBodyComponent", GetComponent<RigidBodyComponent>())
		if (HasComponent<SkinnedModelComponent>())
			SERIALIZE_OBJECT_REF_LBL("SkinnedModelComponent", GetComponent<SkinnedModelComponent>())
		if (HasComponent<BoneComponent>())
			SERIALIZE_OBJECT_REF_LBL("BoneComponent", GetComponent<BoneComponent>())
		if (HasComponent<AudioEmitterComponent>())
			SERIALIZE_OBJECT_REF_LBL("AudioEmitterComponent", GetComponent<AudioEmitterComponent>())
		if (HasComponent<NetScriptComponent>())
			SERIALIZE_OBJECT_REF_LBL("NetScriptComponent", GetComponent<NetScriptComponent>())
		if (HasComponent<NavMeshVolumeComponent>())
			SERIALIZE_OBJECT_REF_LBL("NavMeshVolumeComponent", GetComponent<NavMeshVolumeComponent>())
		if (HasComponent<UIComponent>())
			SERIALIZE_OBJECT_REF_LBL("UIComponent", GetComponent<UIComponent>())
		if(HasComponent<PrefabComponent>())
			SERIALIZE_OBJECT_REF_LBL("PrefabComponent", GetComponent<PrefabComponent>())
		if (HasComponent<PrefabMember>())
			SERIALIZE_OBJECT_REF_LBL("PrefabMember", GetComponent<PrefabMember>())
		END_SERIALIZE();
	}

	bool Entity::Deserialize(const json& j)
	{
		DESERIALIZE_COMPONENT(TransformComponent);
		DESERIALIZE_COMPONENT(VisibilityComponent)
		else
		{
			AddComponent<VisibilityComponent>();
		}
		DESERIALIZE_COMPONENT(NameComponent);
		DESERIALIZE_COMPONENT(ParentComponent);
		
		if (j.contains("PrefabComponent"))
		{
			GetComponent<NameComponent>().IsPrefab = true;
			auto& prefabComp = AddComponent<PrefabComponent>();
			const auto& prefabPath = j["PrefabComponent"]["Path"];

			if (prefabPath.empty())
			{
				return true;
			}

			if (!FileSystem::FileExists(prefabPath, false))
			{
				return true;
			}

			// This will map serialized ID to new generated ones while
			// keeping the parent/child relationship in the prefab file.
			std::string prefabTextContent = FileSystem::ReadFile(prefabPath);
			auto prefabJson = json::parse(prefabTextContent);
			auto& rootId = prefabJson["Root"];
			json rootJson = "";
			for (const auto& entJson : prefabJson["Entities"])
			{
				bool isRoot = false;
				if (entJson["NameComponent"]["ID"] == rootId)
				{
					rootJson = entJson;
					isRoot = true;
					continue;
				}
			}

			Prefab::InstanceOntoRoot(Entity{(entt::entity)GetHandle(), m_Scene}, prefabPath);
			
			DeserializeComponents(rootJson);
		}
		else
		{
			DeserializeComponents(j);
		}
			
		return true;
	}

	bool Entity::DeserializeComponents(const json& j)
	{
		DESERIALIZE_COMPONENT(ParticleEmitterComponent);
		DESERIALIZE_COMPONENT(SpriteComponent);
		DESERIALIZE_COMPONENT(CameraComponent);
		DESERIALIZE_COMPONENT(QuakeMapComponent);
		DESERIALIZE_COMPONENT(LightComponent);
		DESERIALIZE_COMPONENT(ModelComponent);
		DESERIALIZE_COMPONENT(CharacterControllerComponent);
		DESERIALIZE_COMPONENT(BoxColliderComponent);
		DESERIALIZE_COMPONENT(CapsuleColliderComponent);
		DESERIALIZE_COMPONENT(CylinderColliderComponent);
		DESERIALIZE_COMPONENT(MeshColliderComponent);
		DESERIALIZE_COMPONENT(SphereColliderComponent);
		DESERIALIZE_COMPONENT(RigidBodyComponent);
		DESERIALIZE_COMPONENT(BSPBrushComponent);
		DESERIALIZE_COMPONENT(BoneComponent);
		DESERIALIZE_COMPONENT(SkinnedModelComponent);
		DESERIALIZE_COMPONENT(AudioEmitterComponent);
		DESERIALIZE_COMPONENT(NetScriptComponent);
		DESERIALIZE_COMPONENT(NavMeshVolumeComponent);
		DESERIALIZE_COMPONENT(UIComponent);
		return false;
	}

	void Entity::PostDeserialize()
	{
		POSTDESERIALIZE_COMPONENT(QuakeMapComponent);

		if (HasComponent<PrefabComponent>())
		{
			auto& prefabComp = GetComponent<PrefabComponent>();
			const std::string& path = prefabComp.Path;
		}

		POSTDESERIALIZE_COMPONENT(PrefabComponent);
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

	Entity::Entity() 
	{
		this->m_EntityHandle = (entt::entity)-1;
	}
}

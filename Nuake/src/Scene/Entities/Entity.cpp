#include "Entity.h"

#include "src/Scene/Components/ParentComponent.h"
#include "src/Scene/Components/NameComponent.h"
#include "src/Scene/Components/TransformComponent.h"
#include "src/Scene/Components/CameraComponent.h"
#include "src/Scene/Components/QuakeMap.h"
#include "src/Scene/Components/LightComponent.h"
#include "src/Scene/Components/QuakeMap.h"
#include "src/Scene/Components/WrenScriptComponent.h"
#include "src/Scene/Components/CharacterControllerComponent.h"
#include "src/Scene/Components/RigidbodyComponent.h"
#include "src/Scene/Components/BSPBrushComponent.h"
#include "src/Scene/Components/Components.h"
#include "src/Scene/Components/BoxCollider.h"
#include "src/Scene/Components/CapsuleColliderComponent.h"
#include "src/Scene/Components/SpriteComponent.h"
#include "src/Scene/Components/ParticleEmitterComponent.h"

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
		if (HasComponent<WrenScriptComponent>())
			SERIALIZE_OBJECT_REF_LBL("WrenScriptComponent", GetComponent<WrenScriptComponent>())
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
		
		END_SERIALIZE();
	}

	bool Entity::Deserialize(const std::string& str)
	{
		BEGIN_DESERIALIZE();
		DESERIALIZE_COMPONENT(TransformComponent)
		DESERIALIZE_COMPONENT(VisibilityComponent)
		else
		{
			AddComponent<VisibilityComponent>();
		}
		DESERIALIZE_COMPONENT(NameComponent)
		DESERIALIZE_COMPONENT(ParentComponent)
		DESERIALIZE_COMPONENT(ParticleEmitterComponent)
		DESERIALIZE_COMPONENT(SpriteComponent)
		DESERIALIZE_COMPONENT(CameraComponent)
		DESERIALIZE_COMPONENT(QuakeMapComponent)
		DESERIALIZE_COMPONENT(LightComponent)
		DESERIALIZE_COMPONENT(ModelComponent)
		DESERIALIZE_COMPONENT(WrenScriptComponent)
		DESERIALIZE_COMPONENT(CharacterControllerComponent)
		DESERIALIZE_COMPONENT(BoxColliderComponent)
		DESERIALIZE_COMPONENT(CapsuleColliderComponent)
		DESERIALIZE_COMPONENT(CylinderColliderComponent)
		DESERIALIZE_COMPONENT(MeshColliderComponent)
		DESERIALIZE_COMPONENT(SphereColliderComponent)
		DESERIALIZE_COMPONENT(RigidBodyComponent)
		DESERIALIZE_COMPONENT(BSPBrushComponent)
		return true;
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

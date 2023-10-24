#include "SceneNetAPI.h"

#include "Engine.h"
#include "src/Scene/Entities/Entity.h"
#include <src/Scene/Components/ParentComponent.h>
#include <src/Scene/Components/PrefabComponent.h>
#include <src/Scene/Components/CameraComponent.h>
#include <src/Scene/Components/AudioEmitterComponent.h>
#include <src/Scene/Components/ModelComponent.h>
#include <src/Scene/Components/SkinnedModelComponent.h>
#include <src/Scene/Components/BoneComponent.h>
#include <src/Scene/Components/BoxCollider.h>
#include <src/Scene/Components/SphereCollider.h>
#include <src/Scene/Components/CapsuleColliderComponent.h>
#include <src/Scene/Components/CylinderColliderComponent.h>
#include <src/Scene/Components/MeshCollider.h>
#include <src/Scene/Components/CharacterControllerComponent.h>
#include <src/Scene/Components/ParticleEmitterComponent.h>
#include <src/Scene/Components/BSPBrushComponent.h>
#include <src/Scene/Components/SpriteComponent.h>
#include <src/Scene/Components/QuakeMap.h>


namespace Nuake {

	uint32_t GetEntity(Coral::NativeString entityName)
	{
		auto scene = Engine::GetCurrentScene();

		std::string entityNameString = entityName.ToString();
		if (!scene->EntityExists(entityNameString))
		{
			return UINT32_MAX; // Error code: entity not found.
		}

		return scene->GetEntity(entityNameString).GetHandle();
	}

	static enum ComponentTypes
	{
		Unknown = -1,
		PARENT = 0,
		NAME,
		PREFAB,
		TRANSFORM,
		LIGHT,
		CAMERA,
		AUDIO_EMITTER,
		MODEL,
		SKINNED_MODEL,
		BONE,
		RIGIDBODY,
		BOX_COLLIDER,
		SPHERE_COLLIDER,
		CAPSULE_COLLIDER,
		CYLINDER_COLLIDER,
		MESH_COLLIDER,
		CHARACTER_CONTROLLER,
		PARTICLE_EMITTER,
		QUAKE_MAP,
		BSP_BRUSH,
		SPRITE
	};

	bool EntityHasComponent(int id, int componentType)
	{
		uint32_t componentEnumValue = 3;
		//Entity entity = Engine::GetCurrentScene()->GetEntityByID(id);
		Entity entity = { (entt::entity)(id), Engine::GetCurrentScene().get()};

		if (!entity.IsValid())
		{
			return false;
		}

		switch (static_cast<ComponentTypes>(componentEnumValue))
		{
			case PARENT:				return entity.HasComponent<ParentComponent>();
			case NAME:					return entity.HasComponent<NameComponent>();
			case PREFAB:				return entity.HasComponent<PrefabComponent>();
			case TRANSFORM:				return entity.HasComponent<TransformComponent>();
			case LIGHT:					return entity.HasComponent<LightComponent>();
			case CAMERA:				return entity.HasComponent<CameraComponent>();
			case AUDIO_EMITTER:			return entity.HasComponent<AudioEmitterComponent>();
			case MODEL:					return entity.HasComponent<ModelComponent>();
			case SKINNED_MODEL:			return entity.HasComponent<SkinnedModelComponent>();
			case BONE:					return entity.HasComponent<BoneComponent>();
			case BOX_COLLIDER:			return entity.HasComponent<BoxColliderComponent>();
			case SPHERE_COLLIDER:		return entity.HasComponent<SphereColliderComponent>();
			case CAPSULE_COLLIDER:		return entity.HasComponent<CapsuleColliderComponent>();
			case CYLINDER_COLLIDER:		return entity.HasComponent<CylinderColliderComponent>();
			case MESH_COLLIDER:			return entity.HasComponent<MeshColliderComponent>();
			case CHARACTER_CONTROLLER:	return entity.HasComponent<CharacterControllerComponent>();
			case PARTICLE_EMITTER:		return entity.HasComponent<ParticleEmitterComponent>();
			case QUAKE_MAP:				return entity.HasComponent<QuakeMapComponent>();
			case BSP_BRUSH:				return entity.HasComponent<BSPBrushComponent>();
			case SPRITE:				return entity.HasComponent<SpriteComponent>();
			default:
				return false;
		}
	}

	void TransformSetPosition(int entityId, float x, float y, float z)
	{
		Entity entity = { (entt::entity)(entityId), Engine::GetCurrentScene().get() };

		if (entity.IsValid() && entity.HasComponent<TransformComponent>())
		{
			auto& component = entity.GetComponent<TransformComponent>();
			component.SetLocalPosition({ x, y, z });
		}
	}

	void TransformRotate(int entityId, float x, float y, float z)
	{
		Entity entity = { (entt::entity)(entityId), Engine::GetCurrentScene().get() };

		if (entity.IsValid() && entity.HasComponent<TransformComponent>())
		{
			Quat quat = QuatFromEuler(x, y, z);
			auto& component = entity.GetComponent<TransformComponent>();
			component.SetLocalRotation(quat);
		}
	}

	void Nuake::SceneNetAPI::RegisterMethods()
	{
		RegisterMethod("Entity.EntityHasComponentIcall", &EntityHasComponent);
		RegisterMethod("Scene.GetEntityIcall", &GetEntity);

		// Components
		RegisterMethod("TransformComponent.SetPositionIcall", &TransformSetPosition);
		RegisterMethod("TransformComponent.RotateIcall", &TransformRotate);
	}

}


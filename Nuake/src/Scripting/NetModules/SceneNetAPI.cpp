#include "SceneNetAPI.h"

#include "Engine.h"
#include "src/Scene/Entities/Entity.h"
#include "src/Scene/Components/ParentComponent.h"
#include "src/Scene/Components/PrefabComponent.h"
#include "src/Scene/Components/CameraComponent.h"
#include "src/Scene/Components/AudioEmitterComponent.h"
#include "src/Scene/Components/ModelComponent.h"
#include "src/Scene/Components/SkinnedModelComponent.h"
#include "src/Scene/Components/BoneComponent.h"
#include "src/Scene/Components/BoxCollider.h"
#include "src/Scene/Components/SphereCollider.h"
#include "src/Scene/Components/CapsuleColliderComponent.h"
#include "src/Scene/Components/CylinderColliderComponent.h"
#include "src/Scene/Components/MeshCollider.h"
#include "src/Scene/Components/CharacterControllerComponent.h"
#include "src/Scene/Components/ParticleEmitterComponent.h"
#include "src/Scene/Components/BSPBrushComponent.h"
#include "src/Scene/Components/SpriteComponent.h"
#include "src/Scene/Components/QuakeMap.h"

#include "src/Physics/PhysicsManager.h"
#include "src/Scripting/ScriptingEngineNet.h"

#include <Coral/Array.hpp>


namespace Nuake {

	uint32_t GetEntity(Coral::String entityName)
	{
		auto scene = Engine::GetCurrentScene();
		if (!scene->EntityExists(entityName))
		{
			return UINT32_MAX; // Error code: entity not found.
		}

		return scene->GetEntity(entityName).GetHandle();
	}

	Coral::ManagedObject GetEntityScriptFromName(Coral::String entityName)
	{
		auto scene = Engine::GetCurrentScene();
		if (!scene->EntityExists(entityName))
		{
			return Coral::ManagedObject(); // Error code: entity not found.
		}

		Entity entity = scene->GetEntity(entityName);

		auto& scriptingEngine = ScriptingEngineNet::Get();
		if (scriptingEngine.HasEntityScriptInstance(entity))
		{
			auto instance = scriptingEngine.GetEntityScript(entity);
			return instance;
		}
	}

	Coral::ManagedObject GetEntityScriptFromHandle(int entityHandle)
	{
		auto scene = Engine::GetCurrentScene();

		Entity entity = { (entt::entity)(entityHandle), scene.get()};
		if (!entity.IsValid())
		{
			return Coral::ManagedObject(); // Error code: entity not found.
		}

		auto& scriptingEngine = ScriptingEngineNet::Get();
		if (scriptingEngine.HasEntityScriptInstance(entity))
		{
			auto instance = scriptingEngine.GetEntityScript(entity);
			return instance;
		}
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

	bool EntityHasManagedInstance(int handle)
	{
		Entity entity = { (entt::entity)(handle), Engine::GetCurrentScene().get() };
		if (!entity.IsValid())
		{
			return false;
		}

		return ScriptingEngineNet::Get().HasEntityScriptInstance(entity);
	}

	int EntityGetEntity(int handle, Coral::String input)
	{
		std::string path = input;

		Ref<Scene> scene = Engine::GetCurrentScene();
		if (String::BeginsWith(input, "/"))
		{
			return scene->GetEntityFromPath(input).GetHandle();
		}

		Entity entity = { (entt::entity)handle, scene.get() };
		return scene->GetRelativeEntityFromPath(entity, input).GetHandle();
	}

	void TransformSetPosition(int entityId, float x, float y, float z)
	{
		Entity entity = { (entt::entity)(entityId), Engine::GetCurrentScene().get() };

		if (entity.IsValid() && entity.HasComponent<TransformComponent>())
		{
			auto& component = entity.GetComponent<TransformComponent>();
			component.SetLocalPosition({ x, y, z });

			if (entity.HasComponent<CharacterControllerComponent>())
			{
				PhysicsManager::Get().SetCharacterControllerPosition(entity, { x, y, z });
			}
		}
	}

	Coral::Array<float> TransformGetPosition(int entityId)
	{
		Entity entity = { (entt::entity)(entityId), Engine::GetCurrentScene().get() };

		if (entity.IsValid() && entity.HasComponent<TransformComponent>())
		{
			auto& component = entity.GetComponent<TransformComponent>();
			const auto& position = component.GetLocalPosition();
			Coral::Array<float> result = Coral::Array<float>::New({ position.x, position.y, position.z });
			return result;
		}
	}

	Coral::Array<float> TransformGetGlobalPosition(int entityId)
	{
		Entity entity = { (entt::entity)(entityId), Engine::GetCurrentScene().get() };

		if (entity.IsValid() && entity.HasComponent<TransformComponent>())
		{
			auto& component = entity.GetComponent<TransformComponent>();
			const auto& globalPosition = component.GetGlobalPosition();
			Coral::Array<float> result = Coral::Array<float>::New({ globalPosition.x, globalPosition.y, globalPosition.z });
			return result;
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

	Coral::Array<float> CameraGetDirection(int entityId)
	{
		Entity entity = { (entt::entity)(entityId), Engine::GetCurrentScene().get() };

		if (entity.IsValid() && entity.HasComponent<CameraComponent>())
		{
			auto& component = entity.GetComponent<CameraComponent>();
			const Vector3 camDirection = component.CameraInstance->GetDirection();
			return Coral::Array<float>::New({ camDirection.x, camDirection.y, camDirection.z });
		}
	}

	void MoveAndSlide(int entityId, float vx, float vy, float vz)
	{
		Entity entity = { (entt::entity)(entityId), Engine::GetCurrentScene().get() };

		if (entity.IsValid() && entity.HasComponent<CharacterControllerComponent>())
		{
			auto& component = entity.GetComponent<CharacterControllerComponent>();

			if (std::isnan(vx) || std::isnan(vy) || std::isnan(vz))
			{
				return; // Log message here? invalid input
			}

			component.GetCharacterController()->MoveAndSlide({ vx, vy, vz });
		}
	}

	bool IsOnGround(int entityId)
	{
		Entity entity = Entity((entt::entity)(entityId), Engine::GetCurrentScene().get());

		if (entity.IsValid() && entity.HasComponent<CharacterControllerComponent>())
		{
			auto& characterController = entity.GetComponent<CharacterControllerComponent>();
			return PhysicsManager::Get().GetWorld()->IsCharacterGrounded(entity);
		}

		return false;
	}

	void Play(int entityId, Coral::String animation)
	{
		Entity entity = Entity((entt::entity)(entityId), Engine::GetCurrentScene().get());

		if (entity.IsValid() && entity.HasComponent<SkinnedModelComponent>())
		{
			auto& skinnedModel = entity.GetComponent<SkinnedModelComponent>();

			if (skinnedModel.ModelResource)
			{
				auto& model = skinnedModel.ModelResource;

				// Find animation from name
				int animIndex = 0;
				for (const auto& anim : model->GetAnimations())
				{
					if (anim->GetName() == animation)
					{
						model->PlayAnimation(animIndex);
					}

					animIndex++;
				}
			}
		}
	}

	void Nuake::SceneNetAPI::RegisterMethods()
	{
		// Entity
		RegisterMethod("Entity.EntityHasComponentIcall", &EntityHasComponent);
		RegisterMethod("Entity.EntityHasManagedInstanceIcall", &EntityHasManagedInstance);
		RegisterMethod("Entity.EntityGetEntityIcall", &EntityGetEntity);
		// Scene
		RegisterMethod("Scene.GetEntityIcall", &GetEntity);
		RegisterMethod("Scene.GetEntityScriptFromNameIcall", &GetEntityScriptFromName);
		RegisterMethod("Scene.GetEntityScriptFromHandleIcall", &GetEntityScriptFromHandle);

		// Components
		RegisterMethod("TransformComponent.SetPositionIcall", &TransformSetPosition);
		RegisterMethod("TransformComponent.GetPositionIcall", &TransformGetPosition);
		RegisterMethod("TransformComponent.GetGlobalPositionIcall", &TransformGetGlobalPosition);
		RegisterMethod("TransformComponent.RotateIcall", &TransformRotate);

		RegisterMethod("CameraComponent.GetDirectionIcall", &CameraGetDirection);

		RegisterMethod("CharacterControllerComponent.MoveAndSlideIcall", &MoveAndSlide);
		RegisterMethod("CharacterControllerComponent.IsOnGroundIcall", &IsOnGround);

		RegisterMethod("SkinnedModelComponent.PlayIcall", &Play);
	}

}


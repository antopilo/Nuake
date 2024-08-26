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
#include <src/Scene/Components/NavMeshVolumeComponent.h>


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

	int InstancePrefab(Coral::String path)
	{
		if (!FileSystem::FileExists(path))
		{
			return -1;
		}

		const auto& prefab = Prefab::New(path);
		return prefab->Root.GetHandle();
	}

	enum ComponentTypes
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
		SPRITE,
		NAVMESH
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
			case NAVMESH:				return entity.HasComponent<NavMeshVolumeComponent>();
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
		Ref<Scene> scene = Engine::GetCurrentScene();
		if (String::BeginsWith(input, "/"))
		{
			return scene->GetEntityFromPath(input).GetHandle();
		}

		Entity entity = { (entt::entity)handle, scene.get() };
		return scene->GetRelativeEntityFromPath(entity, input).GetHandle();
	}

	Coral::String EntityGetName(int handle)
	{
		Entity entity = { (entt::entity)(handle), Engine::GetCurrentScene().get() };
		if (!entity.IsValid())
		{
			return Coral::String();
		}

		return Coral::String::New(entity.GetComponent<NameComponent>().Name);
	}

	void EntitySetName(int handle, Coral::String newName)
	{
		Entity entity = { (entt::entity)(handle), Engine::GetCurrentScene().get() };
		if (!entity.IsValid())
		{
			return;
		}

		const std::string newEntityName = Engine::GetCurrentScene()->GetUniqueEntityName(newName);
		entity.GetComponent<NameComponent>().Name = newEntityName;
	}

	bool EntityIsValid(int handle)
	{
		Entity entity = { (entt::entity)(handle), Engine::GetCurrentScene().get() };
		return entity.IsValid();
	}

	int PrefabInstance(Coral::String path, Vector3 position, float qx, float qy, float qz, float qw)
	{
		if (!FileSystem::FileExists(path))
		{
			Logger::Log("Prefab path doesn't exist", ".net", CRITICAL);
		}

		const auto& prefab = Prefab::New(path);

		Entity root = prefab->Root;
		TransformComponent& transformComponent = root.GetComponent<TransformComponent>();
		transformComponent.SetLocalPosition(position);
		transformComponent.SetLocalRotation(Quat(qw, qx, qy, qz));

		return root.GetHandle();
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

	float LightGetIntensity(int entityId)
	{
		Logger::Log("Get light intensity with id: " + std::to_string(entityId));

		return -10.0f;
	}

	void LightSetIntensity(int entityId, float intensity)
	{
		Logger::Log("Set light intensity with id: " + std::to_string(intensity));
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

	Coral::Array<float> NavMeshComponentFindPath(int entityId, float startx, float starty, float startz, float endx, float endy, float endz)
	{
		Entity entity = Entity((entt::entity)(entityId), Engine::GetCurrentScene().get());
		if (entity.IsValid() && entity.HasComponent<NavMeshVolumeComponent>())
		{
			auto& navMeshVolume = entity.GetComponent<NavMeshVolumeComponent>();
			if (navMeshVolume.NavMeshData && navMeshVolume.NavMeshData->IsValid())
			{
				auto& navMesh = navMeshVolume.NavMeshData;

				const Vector3& startPosition = { startx, starty, startz };
				const Vector3& endPosition = { endx, endy, endz };
				auto waypoints = navMesh->FindStraightPath(startPosition, endPosition);

				// Convert Vector3 array into float array
				Coral::Array<float> returnResult = Coral::Array<float>::New(static_cast<int32_t>(waypoints.size()) * 3);
				for (int i = 0; i < waypoints.size(); i++)
				{
					returnResult[i * 3 + 0] = waypoints[i].x;
					returnResult[i * 3 + 1] = waypoints[i].y;
					returnResult[i * 3 + 2] = waypoints[i].z;
				}

				return returnResult;
			}
		}

		return {};
	}

	void Nuake::SceneNetAPI::RegisterMethods()
	{
		// Entity
		RegisterMethod("Entity.EntityHasComponentIcall", &EntityHasComponent);
		RegisterMethod("Entity.EntityHasManagedInstanceIcall", &EntityHasManagedInstance);
		RegisterMethod("Entity.EntityGetEntityIcall", &EntityGetEntity);
		RegisterMethod("Entity.EntityGetNameIcall", &EntityGetName);
		RegisterMethod("Entity.EntitySetNameIcall", &EntitySetName);
		RegisterMethod("Entity.EntityIsValidIcall", &EntityIsValid);

		// Prefab
		RegisterMethod("Prefab.PrefabInstanceIcall", &PrefabInstance);

		// Scene
		RegisterMethod("Scene.GetEntityIcall", &GetEntity);
		RegisterMethod("Scene.GetEntityScriptFromNameIcall", &GetEntityScriptFromName);
		RegisterMethod("Scene.GetEntityScriptFromHandleIcall", &GetEntityScriptFromHandle);
		RegisterMethod("Scene.InstancePrefabIcall", &InstancePrefab);

		// Components
		// Transform
		RegisterMethod("TransformComponent.SetPositionIcall", &TransformSetPosition);
		RegisterMethod("TransformComponent.GetPositionIcall", &TransformGetPosition);
		RegisterMethod("TransformComponent.GetGlobalPositionIcall", &TransformGetGlobalPosition);
		RegisterMethod("TransformComponent.RotateIcall", &TransformRotate);

		// Lights
		RegisterMethod("LightComponent.GetLightIntensityIcall", &LightGetIntensity);
		RegisterMethod("LightComponent.SetLightIntensityIcall", &LightSetIntensity);

		// Camera
		RegisterMethod("CameraComponent.GetDirectionIcall", &CameraGetDirection);

		// Character Controller
		RegisterMethod("CharacterControllerComponent.MoveAndSlideIcall", &MoveAndSlide);
		RegisterMethod("CharacterControllerComponent.IsOnGroundIcall", &IsOnGround);

		// Skinned 
		RegisterMethod("SkinnedModelComponent.PlayIcall", &Play);

		// Navigation Mesh
		RegisterMethod("NavMeshVolumeComponent.FindPathIcall", &NavMeshComponentFindPath);
	}

}


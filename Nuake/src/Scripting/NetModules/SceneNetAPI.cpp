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
#include "src/Scene/Systems/PhysicsSystem.h"
#include "src/Scene/Systems/ScriptingSystem.h"
#include "src/Scripting/ScriptingEngineNet.h"

#include <Coral/Array.hpp>
#include <src/Scene/Components/NavMeshVolumeComponent.h>
#include <src/Scene/Components/RigidbodyComponent.h>


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
		Entity entity = { (entt::entity)(id), Engine::GetCurrentScene().get()};

		if (!entity.IsValid())
		{
			return false;
		}

		switch (static_cast<ComponentTypes>(componentType))
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

	void EntityAddComponent(int id, int componentType)
	{
		Entity entity = { (entt::entity)(id), Engine::GetCurrentScene().get() };

		if (!entity.IsValid())
		{
			return;
		}

		switch (static_cast<ComponentTypes>(componentType))
		{
		case PARENT:				entity.AddComponent<ParentComponent>(); break;
			case NAME:					entity.AddComponent<NameComponent>(); break;
			case PREFAB:				entity.AddComponent<PrefabComponent>(); break;
			case TRANSFORM:				entity.AddComponent<TransformComponent>(); break;
			case LIGHT:					entity.AddComponent<LightComponent>(); break;
			case CAMERA:				entity.AddComponent<CameraComponent>(); break;
			case AUDIO_EMITTER:			entity.AddComponent<AudioEmitterComponent>(); break;
			case MODEL:					entity.AddComponent<ModelComponent>(); break;
			case SKINNED_MODEL:			entity.AddComponent<SkinnedModelComponent>(); break;
			case BONE:					entity.AddComponent<BoneComponent>(); break;
			case BOX_COLLIDER:			entity.AddComponent<BoxColliderComponent>(); break;
			case SPHERE_COLLIDER:		entity.AddComponent<SphereColliderComponent>(); break;
			case CAPSULE_COLLIDER:		entity.AddComponent<CapsuleColliderComponent>(); break;
			case CYLINDER_COLLIDER:		entity.AddComponent<CylinderColliderComponent>(); break;
			case MESH_COLLIDER:			entity.AddComponent<MeshColliderComponent>(); break;
			case CHARACTER_CONTROLLER:	entity.AddComponent<CharacterControllerComponent>(); break;
			case PARTICLE_EMITTER:		entity.AddComponent<ParticleEmitterComponent>(); break;
			case QUAKE_MAP:				entity.AddComponent<QuakeMapComponent>(); break;
			case BSP_BRUSH:				entity.AddComponent<BSPBrushComponent>(); break;
			case SPRITE:				entity.AddComponent<SpriteComponent>(); break;
			case NAVMESH:				entity.AddComponent<NavMeshVolumeComponent>(); break;
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

	Coral::String EntityGetTarget(int handle)
	{
		Entity entity = { (entt::entity)(handle), Engine::GetCurrentScene().get() };
		if (entity.IsValid())
		{
			if (entity.HasComponent<BSPBrushComponent>())
			{
				return Coral::String::New(entity.GetComponent<BSPBrushComponent>().target);
			}
		}

		return Coral::String::New("");
	}

	Coral::Array<int> EntityGetTargets(Coral::String target)
	{
		std::vector<int> targetsFound = std::vector<int>();

		if (target == "")
		{
			return Coral::Array<int>::New(targetsFound);
		}

		Ref<Scene> scene = Engine::GetCurrentScene();
		auto brushView = scene->m_Registry.view<BSPBrushComponent>();
		for (auto e : brushView)
		{
			BSPBrushComponent& brushComponent = brushView.get<BSPBrushComponent>(e);
			auto targets = brushComponent.Targets;
			if (brushComponent.TargetName == target)
			{
				Entity entity = { (entt::entity)(e), scene.get() };
				
				targetsFound.push_back(entity.GetHandle());
			}
		}

		return Coral::Array<int>::New(targetsFound);
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

		Ref<Scene> scene = Engine::GetCurrentScene();
		scene->GetPhysicsSystem()->InitializeNewBodies();
		// scene->GetScriptingSystem()->InitializeNewScripts(); // We cant instantiate scripts here..

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

			if (entity.HasComponent<BSPBrushComponent>() || entity.HasComponent<RigidBodyComponent>())
			{
				PhysicsManager::Get().SetBodyTransform(entity, { x, y, z }, component.GetGlobalRotation());
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
			const Vector3& globalPosition = component.GlobalTransform[3];
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

	void TransformLookAt(int entityId, float x, float y, float z, float ux, float uy, float uz)
	{
		Entity entity = { (entt::entity)(entityId), Engine::GetCurrentScene().get() };

		if (entity.IsValid() && entity.HasComponent<TransformComponent>())
		{
			auto& component = entity.GetComponent<TransformComponent>();

			// Get the entity's current position
			const Vector3 globalPos = component.GetGlobalTransform()[3];
			const Vector3 targetPos = { x, y, z };
			const Vector3 direction = glm::normalize(globalPos - targetPos);

			Vector3 up = { ux, uy, uz };
			if (glm::abs(glm::dot(direction, up)) > 0.999f)
			{
				up = { 0.0f, 0.0f, 1.0f };
			}

			// Calculate the right vector
			const Vector3 right = glm::normalize(glm::cross(up, direction));

			// Recompute the true up vector
			const Vector3 adjustedUp = glm::normalize(glm::cross(direction, right));

			// Construct a rotation matrix
			const Matrix3 rotationMatrix = 
			{
				right,
				adjustedUp,
				direction
			};

			// Convert rotation matrix to a quaternion
			Quat orientation = glm::quat_cast(rotationMatrix);
			component.SetLocalRotation(std::move(orientation));
		}
	}

	float LightGetIntensity(int entityId)
	{
		Entity entity = { (entt::entity)(entityId), Engine::GetCurrentScene().get() };
		if (entity.IsValid() && entity.HasComponent<LightComponent>())
		{
			auto& component = entity.GetComponent<LightComponent>();
			return component.Strength;
		}

		return 0.0f;
	}

	void LightSetIntensity(int entityId, float intensity)
	{
		Entity entity = { (entt::entity)(entityId), Engine::GetCurrentScene().get() };
		if (entity.IsValid() && entity.HasComponent<LightComponent>())
		{
			auto& component = entity.GetComponent<LightComponent>();
			component.Strength = intensity;
		}
	}

	void LightSetColor(int entityId, float r, float g, float b)
	{
		Entity entity = { (entt::entity)(entityId), Engine::GetCurrentScene().get() };

		if (entity.IsValid() && entity.HasComponent<LightComponent>())
		{
			auto& component = entity.GetComponent<LightComponent>();
			component.Color = Color(r / 255.0f, g / 255.0f, b / 255.0f, 1.0f);
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

	float CameraGetFOV(int entityId)
	{
		Entity entity = { (entt::entity)(entityId), Engine::GetCurrentScene().get() };

		if (entity.IsValid() && entity.HasComponent<CameraComponent>())
		{
			auto& component = entity.GetComponent<CameraComponent>();
			return component.CameraInstance->Fov;
		}
	}

	void CameraSetFOV(int entityId, float fov)
	{
		float safeFov = glm::clamp(fov, 1.0f, 180.0f);
		Entity entity = { (entt::entity)(entityId), Engine::GetCurrentScene().get() };

		if (entity.IsValid() && entity.HasComponent<CameraComponent>())
		{
			auto& component = entity.GetComponent<CameraComponent>();
			component.CameraInstance->Fov = safeFov;
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

	Coral::Array<float> GetGroundVelocity(int entityId)
	{
		Entity entity = Entity((entt::entity)(entityId), Engine::GetCurrentScene().get());

		Coral::Array<float> resultArray = Coral::Array<float>::New(3);
		if (entity.IsValid() && entity.HasComponent<CharacterControllerComponent>())
		{
			auto& characterController = entity.GetComponent<CharacterControllerComponent>();
			Vector3 groundVelocity = PhysicsManager::Get().GetWorld()->GetCharacterGroundVelocity(entity);

			resultArray[0] = groundVelocity.x;
			resultArray[1] = groundVelocity.y;
			resultArray[2] = groundVelocity.z;
			
			return resultArray;
		}

		resultArray[0] = 0.0f;
		resultArray[1] = 0.0f;
		resultArray[2] = 0.0f;
		return resultArray;
	}

	Coral::Array<float> GetGroundNormal(int entityId)
	{
		Entity entity = Entity((entt::entity)(entityId), Engine::GetCurrentScene().get());

		Coral::Array<float> resultArray = Coral::Array<float>::New(3);
		if (entity.IsValid() && entity.HasComponent<CharacterControllerComponent>())
		{
			auto& characterController = entity.GetComponent<CharacterControllerComponent>();
			Vector3 groundVelocity = PhysicsManager::Get().GetWorld()->GetCharacterGroundNormal(entity);

			resultArray[0] = groundVelocity.x;
			resultArray[1] = groundVelocity.y;
			resultArray[2] = groundVelocity.z;

			return resultArray;
		}

		resultArray[0] = 0.0f;
		resultArray[1] = 0.0f;
		resultArray[2] = 0.0f;
		return resultArray;
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

	bool AudioEmitterGetIsPlaying(int entityId)
	{
		Entity entity = Entity((entt::entity)(entityId), Engine::GetCurrentScene().get());

		if (entity.IsValid() && entity.HasComponent<AudioEmitterComponent>())
		{
			auto& audioEmitter = entity.GetComponent<AudioEmitterComponent>();
			return audioEmitter.IsPlaying;
		}

		return false;
	}

	void AudioEmitterSetIsPlaying(int entityId, Coral::Bool32 isPlaying)
	{
		Entity entity = Entity((entt::entity)(entityId), Engine::GetCurrentScene().get());

		if (entity.IsValid() && entity.HasComponent<AudioEmitterComponent>())
		{
			auto& audioEmitter = entity.GetComponent<AudioEmitterComponent>();
			audioEmitter.IsPlaying = isPlaying;
		}
	}

	void EnvironmentDepthOfFieldSetFocusDistanceIcall(float distance)
	{
		Engine::GetCurrentScene()->GetEnvironment()->DOFFocalDepth = distance;
	}

	float EnvironmentDepthOfFieldGetFocusDistanceIcall()
	{
		return Engine::GetCurrentScene()->GetEnvironment()->DOFFocalDepth;
	}

	void Nuake::SceneNetAPI::RegisterMethods()
	{
		// Entity
		RegisterMethod("Entity.EntityHasComponentIcall", &EntityHasComponent);
		RegisterMethod("Entity.EntityAddComponentIcall", &EntityAddComponent);
		RegisterMethod("Entity.EntityHasManagedInstanceIcall", &EntityHasManagedInstance);
		RegisterMethod("Entity.EntityGetEntityIcall", &EntityGetEntity);
		RegisterMethod("Entity.EntityGetNameIcall", &EntityGetName);
		RegisterMethod("Entity.EntitySetNameIcall", &EntitySetName);
		RegisterMethod("Entity.EntityIsValidIcall", &EntityIsValid);
		RegisterMethod("Entity.EntityGetTargetsIcall", &EntityGetTargets);
		RegisterMethod("Entity.EntityGetTargetIcall", &EntityGetTarget);

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
		RegisterMethod("TransformComponent.LookAtIcall", &TransformLookAt);

		// Lights
		RegisterMethod("LightComponent.GetLightIntensityIcall", &LightGetIntensity);
		RegisterMethod("LightComponent.SetLightIntensityIcall", &LightSetIntensity);
		RegisterMethod("LightComponent.SetLightColorIcall", &LightSetColor);

		// Camera
		RegisterMethod("CameraComponent.GetDirectionIcall", &CameraGetDirection);
		RegisterMethod("CameraComponent.GetCameraFOVIcall", &CameraGetFOV);
		RegisterMethod("CameraComponent.SetCameraFOVIcall", &CameraSetFOV);

		// Character Controller
		RegisterMethod("CharacterControllerComponent.MoveAndSlideIcall", &MoveAndSlide);
		RegisterMethod("CharacterControllerComponent.IsOnGroundIcall", &IsOnGround);
		RegisterMethod("CharacterControllerComponent.GetGroundVelocityIcall", &GetGroundVelocity);
		RegisterMethod("CharacterControllerComponent.GetGroundNormalIcall", &GetGroundNormal);

		// Skinned 
		RegisterMethod("SkinnedModelComponent.PlayIcall", &Play);

		// Navigation Mesh
		RegisterMethod("NavMeshVolumeComponent.FindPathIcall", &NavMeshComponentFindPath);

		// Audio Emitter
		RegisterMethod("AudioEmitterComponent.GetIsPlayingIcall", &AudioEmitterGetIsPlaying);
		RegisterMethod("AudioEmitterComponent.SetIsPlayingIcall", &AudioEmitterSetIsPlaying);

		RegisterMethod("Environment.SetFocusDistanceIcall", &EnvironmentDepthOfFieldSetFocusDistanceIcall);
		RegisterMethod("Environment.GetFocusDistanceIcall", &EnvironmentDepthOfFieldGetFocusDistanceIcall);
	}

}


#pragma once
#include "wren.h"
#include <string>
#include <src/Core/Logger.h>

#include "ScriptModule.h"

#include <iostream>
#include <wren.h>
#include "Engine.h"

#include "src/Core/Physics/PhysicsManager.h"

#include "../Scene/Entities/Entity.h"
#include <src/Scene/Components/TransformComponent.h>
#include <src/Scene/Components/LightComponent.h>
#include <src/Scene/Components/QuakeMap.h>
#include <src/Scene/Components/CameraComponent.h>
#include <src/Scene/Components/RigidbodyComponent.h>
#include <src/Scene/Components/CharacterControllerComponent.h>
#include <src/Scene/Components/WrenScriptComponent.h>
#include <src/Scene/Components/TriggerZone.h>
#include <src/Scene/Components/BSPBrushComponent.h>
#include <src/Scene/Components/PrefabComponent.h>
#include <src/Scene/Components/AudioEmitterComponent.h>

namespace Nuake 
{
	namespace ScriptAPI 
	{
		class SceneModule : public ScriptModule
		{
			std::string GetModuleName() override
			{
				return "Scene";
			}

			void RegisterModule(WrenVM* vm) override
			{
				RegisterMethod("CreateEntity(_)", (void*)CreateEntity);
				RegisterMethod("AddPrefab(_)", (void*)AddPrefab);
				RegisterMethod("GetEntityID(_)", (void*)GetEntity);
				RegisterMethod("EntityHasComponent(_,_)", (void*)EntityHasComponent);

				RegisterMethod("GetTranslation_(_)", (void*)GetTranslation);
				RegisterMethod("GetLocalTranslation_(_)", (void*)GetLocalTranslation);
				RegisterMethod("SetTranslation_(_,_,_,_)", (void*)SetTranslation);
				RegisterMethod("GetRotation_(_)", (void*)GetRotation);
				RegisterMethod("SetRotation_(_,_,_,_)", (void*)SetRotation);
				RegisterMethod("SetLookAt_(_,_,_,_)", (void*)SetLookAt);

				RegisterMethod("SetLightIntensity_(_,_)", (void*)SetLightIntensity);
				RegisterMethod("GetLightIntensity_(_)", (void*)GetLightIntensity);

				RegisterMethod("GetScript_(_)", (void*)GetScript);

				RegisterMethod("SetCameraDirection_(_,_,_,_)", (void*)SetCameraDirection);
				RegisterMethod("GetCameraDirection_(_)", (void*)GetCameraDirection);
				RegisterMethod("GetCameraRight_(_)", (void*)GetCameraRight);

				RegisterMethod("SetAudioEmitterPlaying_(_,_)", (void*)SetAudioEmitterPlaying);

				RegisterMethod("MoveAndSlide_(_,_,_,_)", (void*)MoveAndSlide);
				RegisterMethod("IsCharacterControllerOnGround_(_)", (void*)IsCharacterControllerOnGround);
				RegisterMethod("RayCast_(_,_,_,_,_,_)", (void*)Raycast);

				RegisterMethod("AddForce_(_,_,_,_)", (void*)AddForce);

				RegisterMethod("TriggerGetOverlappingBodyCount_(_)", (void*)TriggerGetOverlappingBodyCount);
				RegisterMethod("TriggerGetOverlappingBodies_(_)", (void*)TriggerGetOverlappingBodies);

				RegisterMethod("BrushGetTargets_(_)", (void*)BrushGetTargets);
				RegisterMethod("BrushGetTargetsCount_(_)", (void*)BrushGetTargetsCount);
			}

			static void CreateEntity(WrenVM* vm)
			{
				const std::string name = wrenGetSlotString(vm, 1);
				const int entity = Engine::GetCurrentScene()->CreateEntity(name).GetHandle();
				wrenSetSlotDouble(vm, 0, entity);
			}

			static void GetEntity(WrenVM* vm)
			{
				std::string name = wrenGetSlotString(vm, 1);
				int handle = Engine::GetCurrentScene()->GetEntity(name).GetHandle();
				wrenSetSlotDouble(vm, 0, handle);
			}

			static void AddPrefab(WrenVM* vm)
			{
				const std::string& prefabPath = wrenGetSlotString(vm, 1);
				if (prefabPath.empty())
				{
					Logger::Log("Cannot add prefab with an empty path.", "script", CRITICAL);
					wrenSetSlotDouble(vm, 0, -1); // -1 is an empty entity.
					return;
				}

				const std::string& prefabName = wrenGetSlotString(vm, 1);
				if (prefabName.empty())
				{
					Logger::Log("Cannot add prefab with an empty name.", "script", CRITICAL);
					wrenSetSlotDouble(vm, 0, -1); // -1 is an empty entity.
					return;
				}

				if (!FileSystem::FileExists(prefabPath))
				{
					Logger::Log("Cannot add prefab. File not found: " + prefabPath, "script", CRITICAL);
					return;
				}

				const auto& prefab = Prefab::New(prefabPath);
				wrenSetSlotDouble(vm, 0, prefab->Root.GetHandle());
			}

			static void EntityHasComponent(WrenVM* vm)
			{
				double handle = wrenGetSlotDouble(vm, 1);
				std::string name = wrenGetSlotString(vm, 2);
				Entity ent = Entity((entt::entity)handle, Engine::GetCurrentScene().get());

				bool result = false;
				if (handle == -1)
				{
					wrenSetSlotBool(vm, 0, result);
					return;
				}

				if (name == "Transform")	result = ent.HasComponent<TransformComponent>();
				if (name == "Light")		result = ent.HasComponent<LightComponent>();
				if (name == "QuakeMap")		result = ent.HasComponent<QuakeMapComponent>();
				if (name == "RigidBody")	result = ent.HasComponent<RigidBodyComponent>();
				if (name == "CharacterController")
				{
					result = ent.HasComponent<CharacterControllerComponent>();
				}
				if (name == "Camera")
				{
					result = ent.HasComponent<CameraComponent>();
				}
				if (name == "Script")
				{
					result = ent.HasComponent<WrenScriptComponent>();
				}
				if (name == "Rigidbody")
				{
					result = ent.HasComponent<RigidBodyComponent>();
				}
				if (name == "Script") {
					result = ent.HasComponent<WrenScriptComponent>();
				}
				if (name == "Brush") {
					result = ent.HasComponent<BSPBrushComponent>();
				}
				if (name == "AudioEmitter") {
					result = ent.HasComponent<AudioEmitterComponent>();
				}

				wrenSetSlotBool(vm, 0, result);
			}

			static void GetScript(WrenVM* vm)
			{
				double handle = wrenGetSlotDouble(vm, 1);
				Entity ent = Entity((entt::entity)handle, Engine::GetCurrentScene().get());

				if (ent.HasComponent<WrenScriptComponent>())
				{
					wrenSetSlotHandle(vm, 0, ent.GetComponent<WrenScriptComponent>().mWrenScript->GetWrenInstanceHandle());
				}
			}

			static void SetLightIntensity(WrenVM* vm)
			{
				double handle = wrenGetSlotDouble(vm, 1);
				double intensity = wrenGetSlotDouble(vm, 2);
				Entity ent = Entity((entt::entity)handle, Engine::GetCurrentScene().get());

				auto& light = ent.GetComponent<LightComponent>();
				light.Strength = static_cast<float>(intensity);
			}

			static void GetLightIntensity(WrenVM* vm)
			{
				double handle = wrenGetSlotDouble(vm, 1);
				double intensity = wrenGetSlotDouble(vm, 2);
				Entity ent = Entity((entt::entity)handle, Engine::GetCurrentScene().get());

				auto& light = ent.GetComponent<LightComponent>();
				wrenSetSlotDouble(vm, 0, light.Strength);
			}

			static void SetCameraDirection(WrenVM* vm)
			{
				double handle = wrenGetSlotDouble(vm, 1);
				double x = wrenGetSlotDouble(vm, 2);
				double y = wrenGetSlotDouble(vm, 3);
				double z = wrenGetSlotDouble(vm, 4);

				Entity ent = Entity((entt::entity)handle, Engine::GetCurrentScene().get());
				auto& cam = ent.GetComponent<CameraComponent>();
				cam.CameraInstance->SetDirection(Vector3(x, y, z));
			}

			static void GetCameraDirection(WrenVM* vm)
			{
				double handle = wrenGetSlotDouble(vm, 1);
				Entity ent = Entity((entt::entity)handle, Engine::GetCurrentScene().get());

				auto& cam = ent.GetComponent<CameraComponent>();

				Vector3 dir = cam.CameraInstance->GetDirection();

				wrenEnsureSlots(vm, 4);

				// set the slots
				// Fill the list
				wrenSetSlotNewList(vm, 0);
				wrenSetSlotDouble(vm, 1, dir.x);
				wrenSetSlotDouble(vm, 2, dir.y);
				wrenSetSlotDouble(vm, 3, dir.z);

				wrenInsertInList(vm, 0, -1, 1);
				wrenInsertInList(vm, 0, -1, 2);
				wrenInsertInList(vm, 0, -1, 3);
			}

			static void GetCameraRight(WrenVM* vm)
			{
				double handle = wrenGetSlotDouble(vm, 1);
				Entity ent = Entity((entt::entity)handle, Engine::GetCurrentScene().get());

				auto& cam = ent.GetComponent<CameraComponent>();

				Vector3 right = cam.CameraInstance->GetRight();

				// set the slots
				wrenSetSlotDouble(vm, 1, right.x);
				wrenSetSlotDouble(vm, 2, right.y);
				wrenSetSlotDouble(vm, 3, right.z);

				// Fill the list
				wrenSetSlotNewList(vm, 0);
				wrenInsertInList(vm, 0, 0, 1);
				wrenInsertInList(vm, 0, 1, 2);
				wrenInsertInList(vm, 0, 2, 3);
			}

			static void SetAudioEmitterPlaying(WrenVM* vm)
			{
				double handle = wrenGetSlotDouble(vm, 1);
				Entity ent = Entity((entt::entity)handle, Engine::GetCurrentScene().get());

				auto& audioEmitter = ent.GetComponent<AudioEmitterComponent>();

				bool isPlaying = wrenGetSlotBool(vm, 2);
				audioEmitter.IsPlaying = isPlaying;
			}

			static void IsCharacterControllerOnGround(WrenVM* vm)
			{
				double handle = wrenGetSlotDouble(vm, 1);
				Entity ent = Entity((entt::entity)handle, Engine::GetCurrentScene().get());
				auto& characterController = ent.GetComponent<CharacterControllerComponent>();
				bool result = PhysicsManager::Get().GetWorld()->IsCharacterGrounded(ent);
				wrenSetSlotBool(vm, 0, result);
			}

			static void Raycast(WrenVM* vm)
			{
				const double& fromX = wrenGetSlotDouble(vm, 1);
				const double& fromY = wrenGetSlotDouble(vm, 2);
				const double& fromZ = wrenGetSlotDouble(vm, 3);
				const auto& from = Vector3(fromX, fromY, fromZ);

				const double& toX = wrenGetSlotDouble(vm, 4);
				const double& toY = wrenGetSlotDouble(vm, 5);
				const double& toZ = wrenGetSlotDouble(vm, 6);
				const auto& to = Vector3(toX, toY, toZ);

				const auto& result = PhysicsManager::Get().GetWorld()->Raycast(from, to);

				// We multiply by 3 since we have 3 floats per hit result: vector3.
				wrenEnsureSlots(vm, static_cast<double>(result.size() * 3));
				wrenSetSlotNewList(vm, 0);

				uint32_t index = 1;
				for (auto& r : result)
				{
					wrenSetSlotDouble(vm, index,	 r.WorldPosition.x);
					wrenSetSlotDouble(vm, index + 1, r.WorldPosition.y);
					wrenSetSlotDouble(vm, index + 2, r.WorldPosition.z);
					wrenInsertInList(vm, 0, -1, index);
					wrenInsertInList(vm, 0, -1, index + 1);
					wrenInsertInList(vm, 0, -1, index + 2);
					index += 3;
				}
			}

			static void MoveAndSlide(WrenVM* vm)
			{
				double handle = wrenGetSlotDouble(vm, 1);
				double x = wrenGetSlotDouble(vm, 2);
				double y = wrenGetSlotDouble(vm, 3);
				double z = wrenGetSlotDouble(vm, 4);

				Entity ent = Entity((entt::entity)handle, Engine::GetCurrentScene().get());
				auto& characterController = ent.GetComponent<CharacterControllerComponent>();
				characterController.GetCharacterController()->MoveAndSlide(Vector3(x, y, z));
			}

			static void AddForce(WrenVM* vm)
			{
				double handle = wrenGetSlotDouble(vm, 1);
				double x = wrenGetSlotDouble(vm, 2);
				double y = wrenGetSlotDouble(vm, 3);
				double z = wrenGetSlotDouble(vm, 4);

				Entity ent = Entity((entt::entity)handle, Engine::GetCurrentScene().get());
				auto& rigidBodyComponent = ent.GetComponent<RigidBodyComponent>();
				rigidBodyComponent.QueuedForce += Vector3(x, y, z);
			}

			static void GetTranslation(WrenVM* vm)
			{
				double handle = wrenGetSlotDouble(vm, 1);
				Entity ent = Entity((entt::entity)handle, Engine::GetCurrentScene().get());
				auto& transform = ent.GetComponent<TransformComponent>();
				// set the slots
				Vector3 position = transform.GetGlobalPosition();
				wrenSetSlotDouble(vm, 1, position.x);
				wrenSetSlotDouble(vm, 2, position.y);
				wrenSetSlotDouble(vm, 3, position.z);

				// Fill the list
				wrenSetSlotNewList(vm, 0);
				wrenInsertInList(vm, 0, 0, 1);
				wrenInsertInList(vm, 0, 1, 2);
				wrenInsertInList(vm, 0, 2, 3);
			}

			static void GetLocalTranslation(WrenVM* vm)
			{
				double handle = wrenGetSlotDouble(vm, 1);
				Entity ent = Entity((entt::entity)handle, Engine::GetCurrentScene().get());
				auto& transform = ent.GetComponent<TransformComponent>();
				// set the slots
				Vector3 position = transform.GetLocalPosition();
				wrenSetSlotDouble(vm, 1, position.x);
				wrenSetSlotDouble(vm, 2, position.y);
				wrenSetSlotDouble(vm, 3, position.z);

				// Fill the list
				wrenSetSlotNewList(vm, 0);
				wrenInsertInList(vm, 0, 0, 1);
				wrenInsertInList(vm, 0, 1, 2);
				wrenInsertInList(vm, 0, 2, 3);
			}

			static void GetRotation(WrenVM* vm)
			{
				double handle = wrenGetSlotDouble(vm, 1);
				Entity ent = Entity((entt::entity)handle, Engine::GetCurrentScene().get());
				auto& transform = ent.GetComponent<TransformComponent>();
				// set the slots
				wrenSetSlotDouble(vm, 1, transform.Rotation.x);
				wrenSetSlotDouble(vm, 2, transform.Rotation.y);
				wrenSetSlotDouble(vm, 3, transform.Rotation.z);

				// Fill the list
				wrenSetSlotNewList(vm, 0);
				wrenInsertInList(vm, 0, 0, 1);
				wrenInsertInList(vm, 0, 1, 2);
				wrenInsertInList(vm, 0, 2, 3);
			}

			static void SetRotation(WrenVM* vm)
			{
				double handle = wrenGetSlotDouble(vm, 1);
				Entity ent = Entity((entt::entity)handle, Engine::GetCurrentScene().get());
				auto& transform = ent.GetComponent<TransformComponent>();
				// set the slots
				float x = (float)wrenGetSlotDouble(vm, 2);
				float y = (float)wrenGetSlotDouble(vm, 3);
				float z = (float)wrenGetSlotDouble(vm, 4);

				transform.SetLocalRotation(QuatFromEuler(x, y, z));
			}

			static void SetLookAt(WrenVM* vm)
			{
				double handle = wrenGetSlotDouble(vm, 1);
				Entity ent = Entity((entt::entity)handle, Engine::GetCurrentScene().get());
				auto& transform = ent.GetComponent<TransformComponent>();
				// set the slots
				float x = (float)wrenGetSlotDouble(vm, 2);
				float y = (float)wrenGetSlotDouble(vm, 3);
				float z = (float)wrenGetSlotDouble(vm, 4);
				Vector3 targetPosition = Vector3(x, y, z);
				Quat rotation = LookAt(transform.GetGlobalPosition(), transform.GetGlobalPosition() + targetPosition);
				transform.SetLocalRotation(rotation);
			}

			static void SetTranslation(WrenVM* vm)
			{
				double handle = wrenGetSlotDouble(vm, 1);
				Entity ent = Entity((entt::entity)handle, Engine::GetCurrentScene().get());
				auto& transform = ent.GetComponent<TransformComponent>();

				// set the slots
				double x = wrenGetSlotDouble(vm, 2);
				double y = wrenGetSlotDouble(vm, 3);
				double z = wrenGetSlotDouble(vm, 4);
				transform.SetLocalPosition(Vector3(x, y, z));
			}

			static void TriggerGetOverlappingBodyCount(WrenVM* vm)
			{
				double handle = wrenGetSlotDouble(vm, 1);
				Entity ent = Entity((entt::entity)handle, Engine::GetCurrentScene().get());

				TriggerZone trigger = ent.GetComponent<TriggerZone>();

				int count = trigger.GetOverLappingCount();

				wrenSetSlotDouble(vm, 0, count);
			}

			static void TriggerGetOverlappingBodies(WrenVM* vm)
			{
				double handle = wrenGetSlotDouble(vm, 1);
				Entity ent = Entity((entt::entity)handle, Engine::GetCurrentScene().get());

				TriggerZone trigger = ent.GetComponent<TriggerZone>();

				std::vector<Entity> entities = trigger.GetOverlappingBodies();

				wrenEnsureSlots(vm, static_cast<int>(entities.size()));

				wrenSetSlotNewList(vm, 0);
				int idx = 1;
				for (auto& e : entities)
				{
					wrenSetSlotDouble(vm, idx, e.GetHandle());
					wrenInsertInList(vm, 0, -1, idx);
					idx++;
				}
			}

			static void BrushGetTargets(WrenVM* vm)
			{
				double handle = wrenGetSlotDouble(vm, 1);
				Entity ent = Entity((entt::entity)handle, Engine::GetCurrentScene().get());

				BSPBrushComponent& brush = ent.GetComponent<BSPBrushComponent>();
				wrenEnsureSlots(vm, static_cast<int>(brush.Targets.size()));
				wrenSetSlotNewList(vm, 0);

				int idx = 1;
				for (auto& e : brush.Targets)
				{
					wrenSetSlotDouble(vm, idx, e.GetHandle());
					wrenInsertInList(vm, 0, -1, idx);
					idx++;
				}
			}

			static void BrushGetTargetsCount(WrenVM* vm) 
			{
				double handle = wrenGetSlotDouble(vm, 1);
				Entity ent = Entity((entt::entity)handle, Engine::GetCurrentScene().get());

				BSPBrushComponent& brush = ent.GetComponent<BSPBrushComponent>();
				wrenSetSlotDouble(vm, 0, static_cast<int>(brush.Targets.size()));
			}
		};
	}
}

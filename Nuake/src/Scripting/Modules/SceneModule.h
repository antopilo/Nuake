#pragma once
#include "wren.h"
#include <string>
#include <src/Core/Logger.h>
#include "ScriptModule.h"
#include <iostream>
#include <wren.h>
#include "Engine.h"
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

namespace Nuake {
	namespace ScriptAPI {
		class SceneModule : public ScriptModule
		{
			std::string GetModuleName() override
			{
				return "Scene";
			}

			void RegisterModule(WrenVM* vm) override
			{
				RegisterMethod("GetEntityID(_)", (void*)GetEntity);
				RegisterMethod("EntityHasComponent(_,_)", (void*)EntityHasComponent);

				RegisterMethod("GetTranslation_(_)", (void*)GetTranslation);
				RegisterMethod("SetTranslation_(_,_,_,_)", (void*)SetTranslation);
				RegisterMethod("GetRotation_(_)", (void*)GetRotation);
				RegisterMethod("SetRotation_(_,_,_,_)", (void*)SetRotation);
				RegisterMethod("SetLightIntensity_(_,_)", (void*)SetLightIntensity);
				RegisterMethod("GetLightIntensity_(_)", (void*)GetLightIntensity);

				RegisterMethod("GetScript_(_)", (void*)GetScript);

				RegisterMethod("SetCameraDirection_(_,_,_,_)", (void*)SetCameraDirection);
				RegisterMethod("GetCameraDirection_(_)", (void*)GetCameraDirection);
				RegisterMethod("GetCameraRight_(_)", (void*)GetCameraRight);

				RegisterMethod("MoveAndSlide_(_,_,_,_)", (void*)MoveAndSlide);
				RegisterMethod("IsCharacterControllerOnGround_(_)", (void*)IsCharacterControllerOnGround);

				RegisterMethod("TriggerGetOverlappingBodyCount_(_)", (void*)TriggerGetOverlappingBodyCount);
				RegisterMethod("TriggerGetOverlappingBodies_(_)", (void*)TriggerGetOverlappingBodies);

				RegisterMethod("BrushGetTargets_(_)", (void*)BrushGetTargets);
				RegisterMethod("BrushGetTargetsCount_(_)", (void*)BrushGetTargetsCount);
			}

			static void GetEntity(WrenVM* vm)
			{
				std::string name = wrenGetSlotString(vm, 1);
				int handle = Engine::GetCurrentScene()->GetEntity(name).GetHandle();
				wrenSetSlotDouble(vm, 0, handle);
			}

			static void EntityHasComponent(WrenVM* vm)
			{
				int handle = wrenGetSlotDouble(vm, 1);
				std::string name = wrenGetSlotString(vm, 2);
				Entity ent = Entity((entt::entity)handle, Engine::GetCurrentScene().get());

				if (name == "Transform")
				{
					bool result = ent.HasComponent<TransformComponent>();
					wrenSetSlotBool(vm, 0, result);
				}
				if (name == "Light")
				{
					bool result = ent.HasComponent<LightComponent>();
					wrenSetSlotBool(vm, 0, result);
				}
				if (name == "QuakeMap")
				{
					bool result = ent.HasComponent<QuakeMapComponent>();
					wrenSetSlotBool(vm, 0, result);
				}
				if (name == "CharacterController")
				{
					bool result = ent.HasComponent<CharacterControllerComponent>();
					wrenSetSlotBool(vm, 0, result);
				}
				if (name == "Camera")
				{
					bool result = ent.HasComponent<CameraComponent>();
					wrenSetSlotBool(vm, 0, result);
				}
				if (name == "Script")
				{
					bool result = ent.HasComponent<CameraComponent>();
					wrenSetSlotBool(vm, 0, result);
				}
				if (name == "Rigidbody")
				{
					bool result = ent.HasComponent<RigidBodyComponent>();
					wrenSetSlotBool(vm, 0, result);
				}
				if (name == "Script") {
					bool result = ent.HasComponent<WrenScriptComponent>();
					wrenSetSlotBool(vm, 0, result);
				}
				if (name == "Brush") {
					bool result = ent.HasComponent<BSPBrushComponent>();
					wrenSetSlotBool(vm, 0, result);
				}
			}

			static void GetScript(WrenVM* vm)
			{
				int handle = wrenGetSlotDouble(vm, 1);
				Entity ent = Entity((entt::entity)handle, Engine::GetCurrentScene().get());

				if (ent.HasComponent<WrenScriptComponent>())
				{
					wrenSetSlotHandle(vm, 0, ent.GetComponent<WrenScriptComponent>().mWrenScript->m_Instance);
				}
			}

			static void SetLightIntensity(WrenVM* vm)
			{
				int handle = wrenGetSlotDouble(vm, 1);
				float intensity = wrenGetSlotDouble(vm, 2);
				Entity ent = Entity((entt::entity)handle, Engine::GetCurrentScene().get());

				auto& light = ent.GetComponent<LightComponent>();
				light.Strength = intensity;
			}

			static void GetLightIntensity(WrenVM* vm)
			{
				int handle = wrenGetSlotDouble(vm, 1);
				float intensity = wrenGetSlotDouble(vm, 2);
				Entity ent = Entity((entt::entity)handle, Engine::GetCurrentScene().get());

				auto& light = ent.GetComponent<LightComponent>();
				wrenSetSlotDouble(vm, 0, light.Strength);
			}

			static void SetCameraDirection(WrenVM* vm)
			{
				int handle = wrenGetSlotDouble(vm, 1);
				float x = wrenGetSlotDouble(vm, 2);
				float y = wrenGetSlotDouble(vm, 3);
				float z = wrenGetSlotDouble(vm, 4);
				Entity ent = Entity((entt::entity)handle, Engine::GetCurrentScene().get());
				auto& cam = ent.GetComponent<CameraComponent>();
				cam.CameraInstance->SetDirection(Vector3(x, y, z));
			}

			static void GetCameraDirection(WrenVM* vm)
			{
				int handle = wrenGetSlotDouble(vm, 1);
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
				int handle = wrenGetSlotDouble(vm, 1);
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

			static void IsCharacterControllerOnGround(WrenVM* vm)
			{
				int handle = wrenGetSlotDouble(vm, 1);
				Entity ent = Entity((entt::entity)handle, Engine::GetCurrentScene().get());
				auto& characterController = ent.GetComponent<CharacterControllerComponent>();
				bool result = characterController.CharacterController->IsOnGround;
				wrenSetSlotBool(vm, 0, result);
			}

			static void MoveAndSlide(WrenVM* vm)
			{
				int handle = wrenGetSlotDouble(vm, 1);
				float x = wrenGetSlotDouble(vm, 2);
				float y = wrenGetSlotDouble(vm, 3);
				float z = wrenGetSlotDouble(vm, 4);

				Entity ent = Entity((entt::entity)handle, Engine::GetCurrentScene().get());
				auto& characterController = ent.GetComponent<CharacterControllerComponent>();
				characterController.CharacterController->MoveAndSlide(Vector3(x, y, z));
			}

			static void GetTranslation(WrenVM* vm)
			{
				int handle = wrenGetSlotDouble(vm, 1);
				Entity ent = Entity((entt::entity)handle, Engine::GetCurrentScene().get());
				auto& transform = ent.GetComponent<TransformComponent>();
				// set the slots
				wrenSetSlotDouble(vm, 1, transform.Translation.x);
				wrenSetSlotDouble(vm, 2, transform.Translation.y);
				wrenSetSlotDouble(vm, 3, transform.Translation.z);

				// Fill the list
				wrenSetSlotNewList(vm, 0);
				wrenInsertInList(vm, 0, 0, 1);
				wrenInsertInList(vm, 0, 1, 2);
				wrenInsertInList(vm, 0, 2, 3);
			}

			static void GetRotation(WrenVM* vm)
			{
				int handle = wrenGetSlotDouble(vm, 1);
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
				int handle = wrenGetSlotDouble(vm, 1);
				Entity ent = Entity((entt::entity)handle, Engine::GetCurrentScene().get());
				auto& transform = ent.GetComponent<TransformComponent>();
				// set the slots
				float x = wrenGetSlotDouble(vm, 2);
				float y = wrenGetSlotDouble(vm, 3);
				float z = wrenGetSlotDouble(vm, 4);

				transform.Rotation = Vector3(x, y, z);
			}

			static void SetTranslation(WrenVM* vm)
			{
				int handle = wrenGetSlotDouble(vm, 1);
				Entity ent = Entity((entt::entity)handle, Engine::GetCurrentScene().get());
				auto& transform = ent.GetComponent<TransformComponent>();
				// set the slots
				float x = wrenGetSlotDouble(vm, 2);
				float y = wrenGetSlotDouble(vm, 3);
				float z = wrenGetSlotDouble(vm, 4);

				transform.Translation = Vector3(x, y, z);
			}

			static void TriggerGetOverlappingBodyCount(WrenVM* vm)
			{
				int handle = wrenGetSlotDouble(vm, 1);
				Entity ent = Entity((entt::entity)handle, Engine::GetCurrentScene().get());

				TriggerZone trigger = ent.GetComponent<TriggerZone>();

				int count = trigger.GetOverLappingCount();

				wrenSetSlotDouble(vm, 0, count);
			}

			static void TriggerGetOverlappingBodies(WrenVM* vm)
			{
				int handle = wrenGetSlotDouble(vm, 1);
				Entity ent = Entity((entt::entity)handle, Engine::GetCurrentScene().get());

				TriggerZone trigger = ent.GetComponent<TriggerZone>();

				std::vector<Entity> entities = trigger.GetOverlappingBodies();

				wrenEnsureSlots(vm, entities.size());

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
				int handle = wrenGetSlotDouble(vm, 1);
				Entity ent = Entity((entt::entity)handle, Engine::GetCurrentScene().get());

				BSPBrushComponent& brush = ent.GetComponent<BSPBrushComponent>();
				wrenEnsureSlots(vm, brush.Targets.size());
				wrenSetSlotNewList(vm, 0);

				int idx = 1;
				for (auto& e : brush.Targets)
				{
					wrenSetSlotDouble(vm, idx, e.GetHandle());
					wrenInsertInList(vm, 0, -1, idx);
					idx++;
				}
			}

			static void BrushGetTargetsCount(WrenVM* vm) {
				int handle = wrenGetSlotDouble(vm, 1);
				Entity ent = Entity((entt::entity)handle, Engine::GetCurrentScene().get());

				BSPBrushComponent& brush = ent.GetComponent<BSPBrushComponent>();
				wrenSetSlotDouble(vm, 0, brush.Targets.size());
			}
		};
	}
}

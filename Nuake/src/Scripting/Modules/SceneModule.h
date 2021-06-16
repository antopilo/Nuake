#pragma once
#include "wren.h"
#include <string>
#include <src/Core/Logger.h>
#include "ScriptModule.h"
#include <iostream>
#include <wren.h>
#include "Engine.h"
#include "../Scene/Entities/Entity.h"
#include <src/Scene/Entities/Components/TransformComponent.h>
#include <src/Scene/Entities/Components/LightComponent.h>
#include <src/Scene/Entities/Components/QuakeMap.h>
#include <src/Scene/Entities/Components/CameraComponent.h>
#include <src/Scene/Entities/Components/RigidbodyComponent.h>
#include <src/Scene/Entities/Components/CharacterControllerComponent.h>

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
			RegisterMethod("GetEntityID(_)", (void*)GetEntity);
			RegisterMethod("EntityHasComponent(_,_)", (void*)EntityHasComponent);
			RegisterMethod("GetTranslation_(_)", (void*)GetTranslation);
			RegisterMethod("SetLightIntensity_(_,_)", (void*)SetLightIntensity);
			RegisterMethod("GetLightIntensity_(_)", (void*)GetLightIntensity);
			RegisterMethod("SetCameraDirection_(_,_,_,_)", (void*)SetCameraDirection);
			RegisterMethod("GetCameraDirection_(_)", (void*)GetCameraDirection);
			RegisterMethod("GetCameraRight_(_)", (void*)GetCameraRight);
			RegisterMethod("MoveAndSlide_(_,_,_,_)", (void*)MoveAndSlide);
			RegisterMethod("IsCharacterControllerOnGround_(_)", (void*)IsCharacterControllerOnGround);
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

			Vector3 right = cam.CameraInstance->cameraRight;

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
			bool result =  characterController.CharacterController->IsOnGround;
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

	};
}
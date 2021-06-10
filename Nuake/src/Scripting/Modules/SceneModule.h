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
	};
}
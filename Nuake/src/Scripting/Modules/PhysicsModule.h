#pragma once
#include "wren.h"
#include <string>
#include <src/Core/Logger.h>
#include "../Core/Maths.h"
#include "ScriptModule.h"
#include <iostream>
#include <wren.h>
#include "../Core/Physics/PhysicsManager.h"

namespace Nuake 
{
	namespace ScriptAPI 
	{
		class PhysicsModule : public ScriptModule
		{
			std::string ModuleName = "Engine";

			std::string GetModuleName() override
			{
				return "Physics";
			}

			void RegisterModule(WrenVM* vm) override
			{
				RegisterMethod("Raycast_(_,_,_,_,_,_)", (void*)Raycast);
			}

			static void Raycast(WrenVM* vm)
			{
				Vector3 v1 = Vector3(wrenGetSlotDouble(vm, 1),
					wrenGetSlotDouble(vm, 2),
					wrenGetSlotDouble(vm, 3));
				Vector3 v2 = Vector3(wrenGetSlotDouble(vm, 4),
					wrenGetSlotDouble(vm, 5),
					wrenGetSlotDouble(vm, 6));

				RaycastResult result = PhysicsManager::Get().Raycast(v1, v2);
				wrenSetSlotNewList(vm, 0);

				// Returns a list with 3 vectors placed sequentially
				// should be reconstructed in the wren side.
				wrenSetSlotDouble(vm, 1, result.LocalPoint.x);
				wrenSetSlotDouble(vm, 2, result.LocalPoint.y);
				wrenSetSlotDouble(vm, 3, result.LocalPoint.z);

				wrenSetSlotDouble(vm, 4, result.WorldPoint.x);
				wrenSetSlotDouble(vm, 5, result.WorldPoint.y);
				wrenSetSlotDouble(vm, 6, result.WorldPoint.z);

				wrenSetSlotDouble(vm, 7, result.Normal.x);
				wrenSetSlotDouble(vm, 8, result.Normal.y);
				wrenSetSlotDouble(vm, 9, result.Normal.z);

				wrenInsertInList(vm, 0, -1, 1);
				wrenInsertInList(vm, 0, -1, 2);
				wrenInsertInList(vm, 0, -1, 3);
				wrenInsertInList(vm, 0, -1, 4);
				wrenInsertInList(vm, 0, -1, 5);
				wrenInsertInList(vm, 0, -1, 6);
				wrenInsertInList(vm, 0, -1, 7);
				wrenInsertInList(vm, 0, -1, 8);
				wrenInsertInList(vm, 0, -1, 9);
			}
		};
	}
}
#pragma once
#include "wren.h"
#include <string>
#include <src/Core/Logger.h>
#include "ScriptModule.h"
#include <iostream>
#include <wren.h>
#include "../Core/Maths.h"

namespace Nuake {
	namespace ScriptAPI {
		class MathModule : public ScriptModule
		{
			std::string ModuleName = "Math";

			std::string GetModuleName() override
			{
				return "Math";
			}

			void RegisterModule(WrenVM* vm) override
			{
				RegisterMethod("Sqrt_(_,_,_)", (void*)Sqrt);
				RegisterMethod("Sin(_)", (void*)Sin);
				RegisterMethod("Cos(_)", (void*)Cos);
				RegisterMethod("Radians(_)", (void*)Radians);
				RegisterMethod("Degrees(_)", (void*)Degrees);
				RegisterMethod("Cross_(_,_,_,_,_,_)", (void*)Cross);
				RegisterMethod("Dot_(_,_,_,_,_,_)", (void*)Dot);
				RegisterMethod("Length_(_,_,_)", (void*)Length);
			}

			static void Sqrt(WrenVM* vm)
			{
				double x = wrenGetSlotDouble(vm, 1);
				double y = wrenGetSlotDouble(vm, 2);
				double z = wrenGetSlotDouble(vm, 3);
				double result = glm::sqrt((x * x) + (y * y) + (z * z));
				wrenSetSlotDouble(vm, 0, result);
			}

			static void Cos(WrenVM* vm)
			{
				double d = wrenGetSlotDouble(vm, 1);
				double result = glm::cos(d);
				wrenSetSlotDouble(vm, 0, result);
			}

			static void Sin(WrenVM* vm)
			{
				double d = wrenGetSlotDouble(vm, 1);
				double result = glm::sin(d);
				wrenSetSlotDouble(vm, 0, result);
			}

			static void Radians(WrenVM* vm)
			{
				double d = wrenGetSlotDouble(vm, 1);
				double result = glm::radians(d);
				wrenSetSlotDouble(vm, 0, result);
			}

			static void Degrees(WrenVM* vm)
			{
				double d = wrenGetSlotDouble(vm, 1);
				double result = glm::degrees(d);
				wrenSetSlotDouble(vm, 0, result);
			}

			static void Cross(WrenVM* vm)
			{
				Vector3 v1 = Vector3(wrenGetSlotDouble(vm, 1),
					wrenGetSlotDouble(vm, 2),
					wrenGetSlotDouble(vm, 3));
				Vector3 v2 = Vector3(wrenGetSlotDouble(vm, 4),
					wrenGetSlotDouble(vm, 5),
					wrenGetSlotDouble(vm, 6));
				Vector3 cross = glm::cross(v1, v2);
			

				wrenSetSlotNewList(vm, 0);
				wrenSetSlotDouble(vm, 1, cross.x);
				wrenSetSlotDouble(vm, 2, cross.y);
				wrenSetSlotDouble(vm, 3, cross.z);

				wrenInsertInList(vm, 0, -1, 1);
				wrenInsertInList(vm, 0, -1, 2);
				wrenInsertInList(vm, 0, -1, 3);
			}

			static void Dot(WrenVM* vm)
			{
				Vector3 v1 = Vector3(wrenGetSlotDouble(vm, 1),
					wrenGetSlotDouble(vm, 2),
					wrenGetSlotDouble(vm, 3));
				Vector3 v2 = Vector3(wrenGetSlotDouble(vm, 4),
					wrenGetSlotDouble(vm, 5),
					wrenGetSlotDouble(vm, 6));
				float dot = glm::dot(v1, v2);

				wrenSetSlotDouble(vm, 0, dot);
			}

			static void Length(WrenVM* vm)
			{
				Vector3 v1 = Vector3(wrenGetSlotDouble(vm, 1),
					wrenGetSlotDouble(vm, 2),
					wrenGetSlotDouble(vm, 3));

				float length = glm::length(v1);
				wrenSetSlotDouble(vm, 0, glm::length(v1));
			}
		};
	}
}
#pragma once
#include "wren.h"
#include <string>
#include <src/Core/Logger.h>
#include "ScriptModule.h"
#include <iostream>
#include <wren.h>
#include "../Core/Maths.h"
#include "../Core/Input.h"

namespace Nuake {
	namespace ScriptAPI {
		class InputModule : public ScriptModule
		{
			std::string GetModuleName() override
			{
				return "Input";
			}

			void RegisterModule(WrenVM* vm) override
			{
				RegisterMethod("GetMouseX()", (void*)GetMouseX);
				RegisterMethod("GetMouseY()", (void*)GetMouseY);

				RegisterMethod("IsKeyDown_(_)", (void*)IsKeyDown);
				RegisterMethod("IsKeyPressed_(_)", (void*)IsKeyPressed);
				RegisterMethod("IsKeyReleased_(_)", (void*)IsKeyReleased);

				RegisterMethod("IsMouseButtonDown_(_)", (void*)IsMouseButtonDown);
				RegisterMethod("IsMouseButtonPressed_(_)", (void*)IsMouseButtonPressed);
				RegisterMethod("IsMouseButtonReleased_(_)", (void*)IsMouseButtonReleased);

				RegisterMethod("HideMouse()", (void*)HideMouse);
				RegisterMethod("ShowMouse()", (void*)ShowMouse);
				RegisterMethod("IsMouseHidden()", (void*)IsMouseHidden);
			}

			static void GetMouseX(WrenVM* vm)
			{
				wrenSetSlotDouble(vm, 0, Input::GetMouseX());
			}

			static void GetMouseY(WrenVM* vm)
			{
				wrenSetSlotDouble(vm, 0, Input::GetMouseY());
			}

			static void IsMouseButtonDown(WrenVM* vm)
			{
				double key = wrenGetSlotDouble(vm, 1);
				bool result = Input::IsMouseButtonDown((int)key);
				wrenSetSlotBool(vm, 0, result);
			}

			static void IsMouseButtonPressed(WrenVM* vm)
			{
				int key = (int)wrenGetSlotDouble(vm, 1);
				bool result = Input::IsMouseButtonPressed(key);
				wrenSetSlotBool(vm, 0, result);
			}

			static void IsMouseButtonReleased(WrenVM* vm)
			{
				int key = (int)wrenGetSlotDouble(vm, 1);
				bool result = Input::IsMouseButtonReleased(key);
				wrenSetSlotBool(vm, 0, result);
			}

			static void IsKeyDown(WrenVM* vm)
			{
				int key = (int)wrenGetSlotDouble(vm, 1);
				bool result = Input::IsKeyDown(key);
				wrenSetSlotBool(vm, 0, result);
			}

			static void IsKeyPressed(WrenVM* vm)
			{
				int key = (int)wrenGetSlotDouble(vm, 1);
				bool result = Input::IsKeyPressed(key);
				wrenSetSlotBool(vm, 0, result);
			}

			static void IsKeyReleased(WrenVM* vm)
			{
				int key = (int)wrenGetSlotDouble(vm, 1);
				bool result = Input::IsKeyReleased(key);
				wrenSetSlotBool(vm, 0, result);
			}

			static void HideMouse(WrenVM* vm)
			{
				Input::HideMouse();
			}

			static void ShowMouse(WrenVM* vm)
			{
				Input::ShowMouse();
			}

			static void IsMouseHidden(WrenVM* vm)
			{
				wrenSetSlotBool(vm, 0, Input::IsMouseHidden());
			}
		};
	}
}
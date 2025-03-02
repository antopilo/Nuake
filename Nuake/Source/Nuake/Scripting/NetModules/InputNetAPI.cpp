#include "InputNetAPI.h"

#include "Nuake/Core/Input.h"

#include <Coral/Array.hpp>
#include <Coral/String.hpp>

namespace Nuake {

	void ShowMouse(bool visible)
	{
		if (visible)
		{
			Input::ShowMouse();
		}
		else
		{
			Input::HideMouse();
		}
	}

	bool IsMouseButtonDown(int keyCode)
	{
		return Input::IsMouseButtonDown(keyCode);
	}

	bool IsKeyDown(int keyCode)
	{
		return Input::IsKeyDown((Key)keyCode);
	}

	bool IsKeyPressed(int keyCode)
	{
		return Input::IsKeyPressed((Key)keyCode);
	}

	Coral::Array<float> GetMousePosition()
	{
		Vector2 mousePosition = Input::GetMousePosition();
		return Coral::Array<float>::New({ mousePosition.x, mousePosition.y });
	}

	bool IsControllerConnected(int id)
	{
		return Input::IsControllerPresent(id);
	}

	Coral::String GetControllerName(int id)
	{
		std::string controllerName = Input::GetControllerName(id);
		return Coral::String::New(controllerName);
	}

	bool IsControllerButtonPressed(int id, int button)
	{
		return Input::IsControllerButtonPressed(id, (ControllerInput)button);
	}

	float GetControllerAxis(int id, int axis)
	{
		return Input::GetControllerAxis(id, (ControllerAxis)axis);
	}

	void InputNetAPI::RegisterMethods()
	{
		RegisterMethod("Input.ShowMouseIcall", (void*)(&ShowMouse));
		RegisterMethod("Input.IsKeyDownIcall", (void*)(&IsKeyDown));
		RegisterMethod("Input.IsKeyPressedIcall", (void*)(&IsKeyPressed));

		RegisterMethod("Input.IsMouseButtonDownIcall", (void*)(&IsMouseButtonDown));
		RegisterMethod("Input.GetMousePositionIcall", (void*)(&GetMousePosition));

		RegisterMethod("Input.IsControllerConnectedIcall", (void*)(&IsControllerConnected));
		RegisterMethod("Input.GetControllerNameIcall", (void*)(&GetControllerName));
		RegisterMethod("Input.IsControllerButtonPressedIcall", (void*)(&IsControllerButtonPressed));
		RegisterMethod("Input.GetControllerAxisIcall", (void*)(&GetControllerAxis));

	}

}


#include "InputNetAPI.h"

#include "src/Core/Input.h"

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
		RegisterMethod("Input.ShowMouseIcall", &ShowMouse);
		RegisterMethod("Input.IsKeyDownIcall", &IsKeyDown);
		RegisterMethod("Input.IsKeyPressedIcall", &IsKeyPressed);

		RegisterMethod("Input.IsMouseButtonDownIcall", &IsMouseButtonDown);
		RegisterMethod("Input.GetMousePositionIcall", &GetMousePosition);

		RegisterMethod("Input.IsControllerConnectedIcall", &IsControllerConnected);
		RegisterMethod("Input.GetControllerNameIcall", &GetControllerName);
		RegisterMethod("Input.IsControllerButtonPressedIcall", &IsControllerButtonPressed);
		RegisterMethod("Input.GetControllerAxisIcall", &GetControllerAxis);

	}

}


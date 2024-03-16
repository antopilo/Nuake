#include "InputNetAPI.h"

#include "src/Core/Input.h"

#include <Coral/NativeArray.hpp>

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

	bool IsKeyDown(int keyCode)
	{
		return Input::IsKeyDown(keyCode);
	}

	bool IsKeyPressed(int keyCode)
	{
		return Input::IsKeyPressed(keyCode);
	}

	Coral::NativeArray<float> GetMousePosition()
	{
		Vector2 mousePosition = Input::GetMousePosition();
		return { mousePosition.x, mousePosition.y};
	}


	void InputNetAPI::RegisterMethods()
	{
		RegisterMethod("Input.ShowMouseIcall", &ShowMouse);
		RegisterMethod("Input.IsKeyDownIcall", &IsKeyDown);
		RegisterMethod("Input.IsKeyPressedIcall", &IsKeyPressed);

		RegisterMethod("Input.GetMousePositionIcall", &GetMousePosition);
	}

}


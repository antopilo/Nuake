#include "InputNetAPI.h"

#include "src/Core/Input.h"

#include <Coral/NativeArray.hpp>

namespace Nuake {

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
		RegisterMethod("Input.IsKeyDownIcall", &IsKeyDown);
		RegisterMethod("Input.IsKeyPressedIcall", &IsKeyPressed);

		RegisterMethod("Input.GetMousePositionIcall", &GetMousePosition);
	}

}


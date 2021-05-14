#pragma once
#include <utility>


class __declspec(dllexport) Input
{
public:
	static bool IsKeyPressed(int keycode);
	static bool IsKeyPress(int keycode);
	static bool IsKeyReleased(int keycode);

	static bool IsMouseButtonPressed(int button);

	static void HideMouse();
	static void ShowMouse();
	static bool IsMouseHidden();

	static float GetMouseX();
	static float GetMouseY();
	static std::pair<float, float> GetMousePosition();

	static bool Init();

	Input* Get() { return s_Instance; }

private:
	static Input* s_Instance;
};

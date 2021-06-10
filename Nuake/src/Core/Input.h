#pragma once
#include <utility>
#include "../Core/Maths.h"
#include <map>
class Input
{
private:
	static bool m_MouseButtons[5];
	static std::map<int, bool> m_Keys;
public:
	static bool IsKeyPressed(int keycode);
	static bool IsKeyDown(int keycode);
	static bool IsKeyReleased(int keycode);

	
	static void HideMouse();
	static void ShowMouse();
	static bool IsMouseHidden();
	static bool IsMouseButtonPressed(int button);
	static bool IsMouseButtonDown(int button);
	static bool IsMouseButtonReleased(int button);

	static float GetMouseX();
	static float GetMouseY();
	static Vector2 GetMousePosition();

	static bool Init();
	static void Update();

	Input* Get() { return s_Instance; }

private:
	static Input* s_Instance;
};

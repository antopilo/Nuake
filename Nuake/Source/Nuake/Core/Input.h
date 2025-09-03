#pragma once
#include "Nuake/Core/Maths.h"

#include <string>
#include <map>
#include <utility>

struct GLFWwindow;

namespace Nuake
{
	enum class Key
	{
		SPACE = 32,
		APOSTROPHE = 39,  /* ' */
		COMMA = 44,  /* , */
		MINUS = 45,  /* - */
		PERIOD = 46, /* . */
		SLASH = 47,  /* / */
		NUM0 = 48,
		NUM1 = 49,
		NUM2 = 50,
		NUM3 = 51,
		NUM4 = 52,
		NUM5 = 53,
		NUM6 = 54,
		NUM7 = 55,
		NUM8 = 56,
		NUM9 = 57,
		SEMICOLON = 59,  /* ; */
		EQUAL = 61, /* = */
		A = 65,
		B = 66,
		C = 67,
		D = 68,
		E = 69,
		F = 70,
		G = 71,
		H = 72,
		I = 73,
		J = 74,
		K = 75,
		L = 76,
		M = 77,
		N = 78,
		O = 79,
		P = 80,
		Q = 81,
		R = 82,
		S = 83,
		T = 84,
		U = 85,
		V = 86,
		W = 87,
		X = 88,
		Y = 89,
		Z = 90,
		LEFT_BRACKET = 91,  /* [ */
		BACKSLASH = 92,  /* \ */
		RIGHT_BRACKET = 93,  /* ] */
		GRAVE_ACCENT = 96,  /* ` */
		WORLD_1 = 161,/* non-US #1 */
		WORLD_2 = 162,/* non-US #2 */
		ESCAPE = 256,
		ENTER = 257,
		TAB = 258,
		BACKSPACE = 259,
		INSERT = 260,
		DELETE_KEY = 261,
		RIGHT = 262,
		LEFT = 263,
		DOWN = 264,
		UP = 265,
		PAGE_UP = 266,
		PAGE_DOWN = 267,
		HOME = 268,
		END = 269,
		CAPS_LOCK = 280,
		SCROLL_LOCK = 281,
		NUM_LOCK = 282,
		PRINT_SCREEN = 283,
		PAUSE = 284,
		F1 = 290,
		F2 = 291,
		F3 = 292,
		F4 = 293,
		F5 = 294,
		F6 = 295,
		F7 = 296,
		F8 = 297,
		F9 = 298,
		F10 = 299,
		F11 = 300,
		F12 = 301,
		F13 = 302,
		F14 = 303,
		F15 = 304,
		F16 = 305,
		F17 = 306,
		F18 = 307,
		F19 = 308,
		F20 = 309,
		F21 = 310,
		F22 = 311,
		F23 = 312,
		F24 = 313,
		F25 = 314,
		KP_0 = 320,
		KP_1 = 321,
		KP_2 = 322,
		KP_3 = 323,
		KP_4 = 324,
		KP_5 = 325,
		KP_6 = 326,
		KP_7 = 327,
		KP_8 = 328,
		KP_9 = 329,
		KP_DECIMAL = 330,
		KP_DIVIDE = 331,
		KP_MULTIPLY = 332,
		KP_SUBTRACT = 333,
		KP_ADD = 334,
		KP_ENTER = 335,
		KP_EQUAL = 336,
		LEFT_SHIFT = 340,
		LEFT_CONTROL = 341,
		LEFT_ALT = 342,
		LEFT_SUPER = 343,
		RIGHT_SHIFT = 344,
		RIGHT_CONTROL = 345,
		RIGHT_ALT = 346,
		RIGHT_SUPER = 347,
		MENU = 348
	};

	enum class ControllerInput
	{
		A = 0, 
		B, 
		X, 
		Y, 
		LEFT_BUMPER, 
		RIGHT_BUMPER, 
		BACK, 
		START, 
		GUIDE, 
		LEFT_THUMB, 
		RIGHT_THUMB, 
		DPAD_UP, 
		DPAD_RIGHT, 
		DPAD_DOWN,
		DPAD_LEFT
	};

	enum class ControllerAxis
	{
		LEFT_X = 0, 
		LEFT_Y, 
		RIGHT_X, 
		RIGHT_Y, 
		LEFT_TRIGGER,
		RIGHT_TRIGGER
	};

	class Input
	{
	private:
		static bool m_MouseButtons[5];
		static std::map<int, bool> m_Keys;
		static Vector2 ViewportPosition;
		static Vector2 ViewportSize;

	public:
		static bool IsKeyPressed(Key keycode);
		static bool IsKeyDown(Key keycode);
		static bool IsKeyReleased(int keycode);

		static float YScroll;
		static float XScroll;
		static void ScrollCallback(GLFWwindow* window, double xoffset, double yoffset);

		static void HideMouse();
		static void ShowMouse();
		static bool IsMouseHidden();
		static bool IsMouseButtonPressed(int button);
		static bool IsMouseButtonDown(int button);
		static bool IsMouseButtonReleased(int button);

		// Get mouse position relative to window
		static float GetMouseX();
		static float GetMouseY();
		static Vector2 GetMousePosition();

		static void SetEditorViewportSize(const Vector2& position, const Vector2& size);
		static Vector2 GetEditorViewportMousePosition();
		static void SetMousePosition(const Vector2& position);

		// Get mouse position relative to viewport
		static float GetViewportMouseY();
		static float GetViewportMouseX();
		static Vector2 GetViewportMousePosition();
		static Vector2 GetViewportSize();
		static void SetViewportDimensions(const Vector2& pos, const Vector2& size);

		// Controller
		static bool IsControllerPresent(int id);
		static std::string GetControllerName(int id);

		static bool IsControllerButtonPressed(int id, ControllerInput input);
		static float GetControllerAxis(int id, ControllerAxis axis);

		static bool Init();
		static void Update();

		Input* Get() { return s_Instance; }

	private:
		static Input* s_Instance;
		static Vector2 s_ViewportPos;
		static Vector2 s_ViewportSize;
	};
}

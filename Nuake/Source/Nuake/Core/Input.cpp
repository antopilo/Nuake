#include "Input.h"
#include "Nuake/Window.h"

#include <imgui/imgui_impl_glfw.h>
#include <GLFW/glfw3.h>
#include <Tracy.hpp>

namespace Nuake
{
	Input* Input::s_Instance;
	Vector2 Input::s_ViewportPos = Vector2(0,0);
	Vector2 Input::s_ViewportSize = Vector2(0, 0);

	std::map<int, bool> Input::m_Keys = std::map<int, bool>();
	bool Input::m_MouseButtons[5] = { false, false, false, false, false };
	float Input::XScroll = 0.0f;
	float Input::YScroll = 0.0f;
	
	Vector2 Input::ViewportPosition = Vector2(0, 0);;
	Vector2 Input::ViewportSize = Vector2(0, 0);
	void Input::ScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
	{
		XScroll = (float)xoffset;
		YScroll = (float)yoffset;

		ImGui_ImplGlfw_ScrollCallback(window, xoffset, yoffset);
	}

#pragma region Keys
	// Only true if the key is currently being pressed
	bool Input::IsKeyDown(Key keycode)
	{
		auto window = Window::Get()->GetHandle();
		int state = glfwGetKey(window, (int)keycode);
		bool result = state == GLFW_PRESS;

		return result;
	}

	// Only true if the key is pressed for the first frame. no repeat.
	bool Input::IsKeyPressed(Key keycode)
	{
		auto window = Window::Get()->GetHandle();
		int state = glfwGetKey(window, (int)keycode);
		bool result = state == GLFW_PRESS;

		// First time pressed?
		if (m_Keys.find((int)keycode) == m_Keys.end() || m_Keys[(int)keycode] == false)
		{
			if (result)
				m_Keys[(int)keycode] = true;

			return result;
		}

		return false;
	}


	bool Input::IsKeyReleased(int keycode)
	{
		auto window = Window::Get()->GetHandle();
		int state = glfwGetKey(window, keycode);
		bool result = state == GLFW_RELEASE;

		// First time pressed?
		if (m_Keys.find(keycode) != m_Keys.end())
		{
			return result && m_Keys[keycode] == true;
		}

		return false;
	}
#pragma endregion

#pragma region Mouse

	// Visibility
	void Input::HideMouse() 
	{
		auto window = Window::Get()->GetHandle();
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NoMouse;
	}

	bool Input::IsMouseHidden() 
	{
		auto window = Window::Get()->GetHandle();
		return glfwGetInputMode(window, GLFW_CURSOR) == GLFW_CURSOR_DISABLED;
	}

	void Input::ShowMouse() 
	{
		auto window = Window::Get()->GetHandle();
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		ImGui::GetIO().ConfigFlags &= ~ImGuiConfigFlags_NoMouse;
	}

	// Action
	bool Input::IsMouseButtonDown(int button)
	{
		auto window = Window::Get()->GetHandle();
		auto state = glfwGetMouseButton(window, button);

		return state == GLFW_PRESS;
	}

	bool Input::IsMouseButtonPressed(int button)
	{
		auto window = Window::Get()->GetHandle();
		auto state = glfwGetMouseButton(window, button);

		if (m_MouseButtons[button] == false && state == GLFW_PRESS)
		{
			m_MouseButtons[button] = true;
			return true;
		}

		return false;
	}

	bool Input::IsMouseButtonReleased(int button)
	{
		auto window = Window::Get()->GetHandle();
		auto state = glfwGetMouseButton(window, button);
		return state == GLFW_RELEASE && m_MouseButtons[button] == true;
	}

	// Position
	float Input::GetMouseX()
	{
		auto window = Window::Get()->GetHandle();

		double xpos, ypos;
		glfwGetCursorPos(window, &xpos, &ypos);

		return (float)xpos;
	}

	float Input::GetMouseY()
	{
		auto window = Window::Get()->GetHandle();

		double xpos, ypos;
		glfwGetCursorPos(window, &xpos, &ypos);

		return (float)ypos;
	}

	Vector2 Input::GetMousePosition()
	{
		auto window = Window::Get()->GetHandle();

		double xpos, ypos;
		glfwGetCursorPos(window, &xpos, &ypos);

		return Vector2(xpos, ypos);
	}

	void Input::SetEditorViewportSize(const Vector2& position, const Vector2& size)
	{
		s_ViewportPos = position;
		s_ViewportSize = size;
	}

	Vector2 Input::GetEditorViewportMousePosition()
	{
		if (s_ViewportPos == s_ViewportSize)
		{
			return GetMousePosition();
		}

		auto window = Window::Get()->GetHandle();

		double xpos, ypos;
		glfwGetCursorPos(window, &xpos, &ypos);

		Vector2 absolutePosition = { xpos, ypos };
		Vector2 offset = absolutePosition - ViewportPosition;

		return Vector2(xpos , ypos);
	}

	void Input::SetMousePosition(const Vector2& position)
	{
		auto window = Window::Get()->GetHandle();
		glfwSetCursorPos(window, position.x, position.y);
	}

	float Input::GetViewportMouseY() 
	{
		return glm::clamp(GetMouseY() - ViewportPosition.y, 0.0f, ViewportSize.y);
	}

	float Input::GetViewportMouseX() 
	{
		return glm::clamp(GetMouseX() - ViewportPosition.x, 0.0f, ViewportSize.x);
	}

	Vector2 Input::GetViewportMousePosition()
	{
		return glm::clamp(GetMousePosition() - ViewportPosition, { 0, 0 }, ViewportSize);
	}

	Vector2 Input::GetViewportSize()
	{
		return ViewportSize;
	}

	void Input::SetViewportDimensions(const Vector2& pos, const Vector2& size)
	{
		ViewportPosition = pos;
		ViewportSize = size;
	}

#pragma endregion

	bool Input::IsControllerPresent(int id)
	{
		if (id > GLFW_JOYSTICK_LAST)
		{
			return 0;
		}
		return glfwJoystickPresent(GLFW_JOYSTICK_1 + id);
	}

	std::string Input::GetControllerName(int id)
	{
		if (IsControllerPresent(id))
		{
			return glfwGetJoystickName(id);
		}

		return "Controller Disconnected";
	}

	bool Input::IsControllerButtonPressed(int id, ControllerInput input)
	{
		GLFWgamepadstate state;
		if (glfwGetGamepadState(GLFW_JOYSTICK_1 + id, &state))
		{
			if (state.buttons[static_cast<int>(input)])
			{
				return true;
			}
		}

		return false;
	}

	float Input::GetControllerAxis(int id, ControllerAxis axis)
	{
		GLFWgamepadstate state;
		if (glfwGetGamepadState(GLFW_JOYSTICK_1 + id, &state))
		{
			return state.axes[static_cast<int>(axis)];
		}

		return 0.0f;
	}

	bool Input::Init()
	{
		//auto window = Application::Get().GetWindow()->GetNative();
		//glfwSetKeyCallback(window, Input::HandleInputCallback);
		glfwSetScrollCallback(Window::Get()->GetHandle(), Input::ScrollCallback);
		return false;
	}

	void Input::Update()
	{
		ZoneScoped;

		// Reset all input to false.
		for (auto& k : m_Keys)
		{
			if (!IsKeyDown((Key)k.first))
			{
				k.second = false;
			}
		}

		for (int i = 0; i < 5; i++)
		{
			if (!IsMouseButtonDown(i))
			{
				m_MouseButtons[i] = false;
			}
		}
	}
}




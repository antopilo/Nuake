#pragma once

#include "InputManager.h"

#include <src/Window.h>

#include <iostream>
#include <GLFW/glfw3.h>

namespace Nuake
{
	class MyInputManager : public NuakeUI::InputManager
	{
	public:
		MyInputManager(Window& window)
		{
			mWindow = window.GetHandle();

			glfwSetScrollCallback(mWindow, MyInputManager::scroll_callback);
			glfwSetCharCallback(mWindow, MyInputManager::char_callback);
			glfwSetKeyCallback(mWindow, MyInputManager::key_callback);
		}

		bool IsMouseInputDown() override
		{
			auto state = glfwGetMouseButton(mWindow, GLFW_MOUSE_BUTTON_1);
			return state == GLFW_PRESS;
		}

		bool IsKeyPressed(uint32_t key) override
		{
			auto states = glfwGetKey(mWindow, key);
			return states == GLFW_PRESS;
		}

		float GetMouseX() override
		{
			double xpos, ypos;
			glfwGetCursorPos(mWindow, &xpos, &ypos);
			return (float)xpos;
		}

		float GetMouseY() override
		{
			double xpos, ypos;
			glfwGetCursorPos(mWindow, &xpos, &ypos);
			return (float)ypos;
		}

		float GetScrollX() override
		{
			return ScrollX;
		}

		float GetScrollY() override
		{
			return ScrollY;
		}

		static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
		{
			ScrollX = (float)xoffset;
			ScrollY = (float)yoffset;
		}

		static void char_callback(GLFWwindow* window, unsigned int codepoint)
		{
			char c[5] = { 0x00,0x00,0x00,0x00,0x00 };
			if (codepoint <= 0x7F) { c[0] = codepoint; }
			else if (codepoint <= 0x7FF) { c[0] = (codepoint >> 6) + 192; c[1] = (codepoint & 63) + 128; }
			else if (0xd800 <= codepoint && codepoint <= 0xdfff) {} //invalid block of utf8
			else if (codepoint <= 0xFFFF) { c[0] = (codepoint >> 12) + 224; c[1] = ((codepoint >> 6) & 63) + 128; c[2] = (codepoint & 63) + 128; }
			else if (codepoint <= 0x10FFFF) { c[0] = (codepoint >> 18) + 240; c[1] = ((codepoint >> 12) & 63) + 128; c[2] = ((codepoint >> 6) & 63) + 128; c[3] = (codepoint & 63) + 128; }

			InputStack.push(c);
		}

		static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
		{
			if (key == GLFW_KEY_BACKSPACE && (action == GLFW_REPEAT || action == GLFW_PRESS))
			{
				InputStack.push("_backspace");
			}
		}

	private:
		GLFWwindow* mWindow;
	};
}
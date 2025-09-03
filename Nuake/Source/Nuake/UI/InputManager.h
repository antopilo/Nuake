#pragma once
#include <stack>
#include <string>

namespace NuakeUI
{
	class InputManager
	{
	public:
		static std::stack<std::string> InputStack;
		
		static float ScrollX;
		static float ScrollY;

		virtual bool IsMouseInputDown() = 0;
		virtual float GetMouseX() = 0;
		virtual float GetMouseY() = 0;

		virtual float GetScrollX() = 0;
		virtual float GetScrollY() = 0;

		virtual bool IsKeyPressed(uint32_t key) = 0;

		static std::string ConsumeStack()
		{
			std::string item = InputStack.top();
			InputStack.pop();
			return item;
		}
	};
}
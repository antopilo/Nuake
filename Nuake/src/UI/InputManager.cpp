#include "InputManager.h"

namespace NuakeUI
{
	float InputManager::ScrollX = 0.f;
	float InputManager::ScrollY = 0.f;
	std::stack<std::string> InputManager::InputStack = std::stack<std::string>();
}
#include "Input.h"
#include "../Window.h"
#include <GLFW/glfw3.h>

Input* Input::s_Instance;
// TODO: Key press, and release system.

bool Input::IsKeyPressed(int keycode)
{
	auto window = Window::Get()->GetHandle();

	int state = glfwGetKey(window, keycode);

	return state == GLFW_PRESS || state == GLFW_REPEAT;
}

bool Input::IsKeyPress(int keycode)
{
	auto window = Window::Get()->GetHandle();
	int state = glfwGetKey(window, keycode);
	return state == GLFW_PRESS;
}

bool Input::IsKeyReleased(int keycode)
{
	auto window = Window::Get()->GetHandle();
	int state = glfwGetKey(window, keycode);
	return state == GLFW_RELEASE;
}

void Input::HideMouse() {
	auto window = Window::Get()->GetHandle();
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}



// TODO: Rename
bool Input::IsMouseHidden() {
	auto window = Window::Get()->GetHandle();
	return glfwGetInputMode(window, GLFW_CURSOR) == GLFW_CURSOR_DISABLED;
}

void Input::ShowMouse() {
	auto window = Window::Get()->GetHandle();
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
}

bool Input::IsMouseButtonPressed(int button)
{
	auto window = Window::Get()->GetHandle();
	auto state = glfwGetMouseButton(window, button);

	return state == GLFW_PRESS;
}

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

std::pair<float, float> Input::GetMousePosition()
{
	auto window = Window::Get()->GetHandle();

	double xpos, ypos;
	glfwGetCursorPos(window, &xpos, &ypos);

	return { (float)xpos, (float)ypos };
}

bool Input::Init()
{
	//auto window = Application::Get().GetWindow()->GetNative();
	//glfwSetKeyCallback(window, Input::HandleInputCallback);
	return false;
}



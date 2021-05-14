#include "EditorCamera.h"
#include "../Core/Input.h"
#include <GLFW/glfw3.h>
#include <glm/trigonometric.hpp>

void EditorCamera::Update(Timestep ts)
{
	float x = Input::GetMouseX();
	float y = Input::GetMouseY();

	if (!controlled && Input::IsMouseButtonPressed(1))
	{
		mouseLastX = x;
		mouseLastY = y;
	}

	controlled = Input::IsMouseButtonPressed(1);

	if (!controlled)
		Input::ShowMouse();
	else
		Input::HideMouse();

	if (Input::IsKeyPress(GLFW_KEY_UP))
		Speed += 0.1f;
	else if (Input::IsKeyPress(GLFW_KEY_DOWN))
		Speed -= 0.1f;
	if (Speed < 0)
		Speed = 0;

	// Keyboard
	if (!controlled) {
		return;
	}

	if (m_Type == CAMERA_TYPE::ORTHO) {
		if (Input::IsKeyPressed(GLFW_KEY_RIGHT))
			Translation.x += Speed * ts;
		if (Input::IsKeyPressed(GLFW_KEY_LEFT))
			Translation.x -= Speed * ts;

		if (Input::IsKeyPressed(GLFW_KEY_UP))
			Translation.y += Speed * ts;
		if (Input::IsKeyPressed(GLFW_KEY_DOWN))
			Translation.y -= Speed * ts;
	}
	else {
		glm::vec3 movement = glm::vec3(0, 0, 0);

		if (Input::IsKeyPressed(GLFW_KEY_D))
			movement -= cameraRight * (Speed * ts);
		if (Input::IsKeyPressed(GLFW_KEY_A))
			movement += cameraRight * (Speed * ts);

		if (Input::IsKeyPressed(GLFW_KEY_W))
			movement += cameraDirection * (Speed * ts);
		if (Input::IsKeyPressed(GLFW_KEY_S))
			movement -= cameraDirection * (Speed * ts);
		if (Input::IsKeyPressed(GLFW_KEY_LEFT_SHIFT))
			movement -= up * (Speed * ts);
		if (Input::IsKeyPressed(GLFW_KEY_SPACE))
			movement += up * (Speed * ts);


		Translation += glm::vec3(movement.x, 0, 0);

		Translation += glm::vec3(0, movement.y, 0);

		Translation += glm::vec3(0, 0, movement.z);
	}


	if (firstMouse)
	{
		mouseLastX = x;
		mouseLastY = y;
		firstMouse = false;
	}

	// mouse
	float diffx = x - mouseLastX;
	float diffy = mouseLastY - y;
	mouseLastX = x;
	mouseLastY = y;

	const float sensitivity = 0.1f;
	diffx *= sensitivity;
	diffy *= sensitivity;

	Yaw += diffx;
	Pitch += diffy;

	if (Pitch > 89.0f)
		Pitch = 89.0f;
	if (Pitch < -89.0f)
		Pitch = -89.0f;

	cameraDirection.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
	cameraDirection.y = sin(glm::radians(Pitch));
	cameraDirection.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
	cameraFront = glm::normalize(cameraDirection);
	cameraRight = glm::normalize(glm::cross(up, cameraFront));
}
#include "EditorCamera.h"
#include "../Core/Input.h"
#include <GLFW/glfw3.h>
#include <glm/trigonometric.hpp>
#include "src/Core/Logger.h"

namespace Nuake
{
	void EditorCamera::Update(Timestep ts)
	{
		float x = Input::GetMouseX();
		float y = Input::GetMouseY();

		if (!controlled && Input::IsMouseButtonDown(1))
		{
			mouseLastX = x;
			mouseLastY = y;
		}

		controlled = Input::IsMouseButtonDown(1);

		//if (!controlled)
		//	Input::ShowMouse();
		//else
		//	Input::HideMouse();

		// Should probably not have speed binding in here.
		if (Input::IsKeyDown(GLFW_KEY_UP))
			Speed += 0.1f;
		else if (Input::IsKeyDown(GLFW_KEY_DOWN))
			Speed -= 0.1f;

		if (Speed < 0)
			Speed = 0;

		// Keyboard
		if (!controlled)
			return;

		if (m_Type == CAMERA_TYPE::ORTHO) 
		{
			if (Input::IsKeyDown(GLFW_KEY_RIGHT))
				Translation.x += Speed * ts;
			if (Input::IsKeyDown(GLFW_KEY_LEFT))
				Translation.x -= Speed * ts;
			if (Input::IsKeyDown(GLFW_KEY_UP))
				Translation.y += Speed * ts;
			if (Input::IsKeyDown(GLFW_KEY_DOWN))
				Translation.y -= Speed * ts;
		}
		else 
		{
			glm::vec3 movement = glm::vec3(0, 0, 0);

			if (Input::IsKeyDown(GLFW_KEY_D))
				movement -= Right * (Speed * ts);
			if (Input::IsKeyDown(GLFW_KEY_A))
				movement += Right * (Speed * ts);

			if (Input::IsKeyDown(GLFW_KEY_W))
				movement += Direction * (Speed * ts);
			if (Input::IsKeyDown(GLFW_KEY_S))
				movement -= Direction * (Speed * ts);
			if (Input::IsKeyDown(GLFW_KEY_LEFT_SHIFT))
				movement -= Up * (Speed * ts);
			if (Input::IsKeyDown(GLFW_KEY_SPACE))
				movement += Up * (Speed * ts);

			Translation += Vector3(movement);
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

		Direction.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
		Direction.y = sin(glm::radians(Pitch));
		Direction.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
		Direction = glm::normalize(Direction);
		Right = glm::normalize(glm::cross(Up, Direction));
	}
	Ref<EditorCamera> EditorCamera::Copy()
	{
		Ref<EditorCamera> copy = CreateRef<EditorCamera>();
		copy->Translation = this->Translation;
		copy->Yaw = this->Yaw;
		copy->Pitch = this->Pitch;

		return copy;
	}
}

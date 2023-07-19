#include "EditorCamera.h"
#include "../Core/Input.h"
#include <GLFW/glfw3.h>
#include <glm/trigonometric.hpp>
#include "src/Core/Logger.h"

namespace Nuake
{
	void EditorCamera::Update(Timestep ts, const bool hover)
	{
		if (IsMoving)
		{
			Translation = glm::mix(Translation, TargetPos, 3.1f * ts);
			Direction = glm::mix(Direction, TargetPos - Translation, 3.1f * ts);
			if (glm::length(Translation - TargetPos) < 3.0f)
			{
				IsMoving = false;
				Translation = TargetPos - glm::normalize((TargetPos - Translation)) * 3.0f;
				Direction = TargetPos - Translation;
			}
		}

		const float x = Input::GetMouseX();
		const float y = Input::GetMouseY();

		const bool isFlying = Input::IsMouseButtonDown(1) || Input::IsMouseButtonDown(2);
		const bool isPressingMouse = isFlying || Input::YScroll != 0.0f;
		
		if (hover)
		{
			bool previous = controlled;
			controlled = isPressingMouse;

			if (!previous && controlled)
			{
				firstMouse = true;
				mouseLastX = x;
				mouseLastY = y;
			}
		}
		else
		{
			if (controlled && !isPressingMouse)
			{
				controlled = false;
				firstMouse = true;
			}
		}

		if (!controlled)
		{
			Input::ShowMouse();
		}
		else if(hover)
		{
			Input::HideMouse();
		}

		if (Input::IsKeyDown(GLFW_KEY_LEFT_ALT))
		{
			if (Input::YScroll != 0.0f) 
			{
				this->Fov += Input::YScroll;
				Fov = glm::max(Fov, 5.0f);
				Input::YScroll = 0.0f;
			}
		}
		else
		{
			// Keyboard
			if (!controlled)
			{
				mouseLastX = x;
				mouseLastY = y;
				return;
			}

			if (Input::IsMouseButtonDown(1))
			{
				// Should probably not have speed binding in here.
				if (Input::YScroll != 0)
				{
					Speed += Input::YScroll;
					Input::YScroll = 0.0f;
				}

				if (Speed < 0)
					Speed = 0;

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
					auto movement = Vector3(0, 0, 0);

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

				SetDirection(glm::normalize(Direction));
				Right = glm::normalize(glm::cross(Up, Direction));
			}
			else if (Input::IsMouseButtonDown(2))
			{
				Vector3 movement = Vector3(0);
				const float deltaX = x - mouseLastX;
				const float deltaY = y - mouseLastY;
				movement += Right * (deltaX * ts);
				movement += Up * (deltaY * ts);
				Translation += Vector3(movement) * 0.5f;

				mouseLastX = x;
				mouseLastY = y;
				controlled = true;

				SetDirection(glm::normalize(Direction));
			}
			else if (Input::YScroll != 0)
			{
				Translation += Vector3(Direction) * Input::YScroll;
				Input::YScroll = 0.0f;
			}

			SetDirection(glm::normalize(Direction));
			mouseLastX = x;
			mouseLastY = y;
		}
	}

	Ref<EditorCamera> EditorCamera::Copy()
	{
		Ref<EditorCamera> copy = CreateRef<EditorCamera>();
		copy->Translation = this->Translation;
		copy->Yaw = this->Yaw;
		copy->Pitch = this->Pitch;

		return copy;
	}

	void EditorCamera::UpdateDirection()
	{
		Direction.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
		Direction.y = sin(glm::radians(Pitch));
		Direction.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
		Direction = glm::normalize(Direction);
		SetDirection(Direction);

	}

	void EditorCamera::SetYaw(float yaw)
	{
		Yaw = yaw;
		UpdateDirection();
	}

	void EditorCamera::SetPitch(float pitch)
	{
		Pitch = pitch;
		UpdateDirection();
	}

	json EditorCamera::Serialize()
	{
		BEGIN_SERIALIZE();

		SERIALIZE_VEC3(Translation);
		j["Yaw"] = Yaw;
		j["Pitch"] = Pitch;

		return j;
	}

	bool EditorCamera::Deserialize(const std::string& str)
	{
		BEGIN_DESERIALIZE();

		DESERIALIZE_VEC3(j["Translation"], Translation);
		SetYaw(j["Yaw"]);
		SetPitch(j["Pitch"]);

		return true;
	}
}

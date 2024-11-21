#include "EditorCamera.h"
#include "../Core/Input.h"
#include <GLFW/glfw3.h>
#include <glm/trigonometric.hpp>

#include "src/Core/Logger.h"
#include <src/Vendors/imgui/imgui.h>
#include <Engine.h>
#include "src/Resource/Project.h"

namespace Nuake
{
	bool EditorCamera::Update(Timestep ts, const bool hover)
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

		m_IsFlying = Input::IsMouseButtonDown(1) || Input::IsMouseButtonDown(2);

		const bool isPressingMouse = m_IsFlying || Input::YScroll != 0.0f;
		
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



		const bool smoothCamera = Engine::GetProject()->Settings.SmoothCamera;
		const float smoothCameraSpeed = Engine::GetProject()->Settings.SmoothCameraSpeed;
		Yaw = glm::mix(Yaw, TargetYaw, smoothCamera ? smoothCameraSpeed : 1.0f);
		Pitch = glm::mix(Pitch, TargetPitch, smoothCamera ? smoothCameraSpeed : 1.0f);

		

		if (Input::IsKeyDown(Key::LEFT_ALT))
		{
			if (Input::YScroll != 0.0f) 
			{
				this->Fov += Input::YScroll;
				Fov = glm::max(Fov, 5.0f);
				Input::YScroll = 0.0f;
			}

			return true;
		}

		// Keyboard
		if (!controlled)
		{
			mouseLastX = x;
			mouseLastY = y;
		}

		
		if (controlled && Input::IsMouseButtonDown(1))
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
				if (Input::IsKeyDown(Key::RIGHT))
					Translation.x += Speed * ts;
				if (Input::IsKeyDown(Key::LEFT))
					Translation.x -= Speed * ts;
				if (Input::IsKeyDown(Key::UP))
					Translation.y += Speed * ts;
				if (Input::IsKeyDown(Key::DOWN))
					Translation.y -= Speed * ts;
			}
			else
			{
				auto movement = Vector3(0, 0, 0);

				if (Input::IsKeyDown(Key::D))
					movement -= Right * (Speed * ts);
				if (Input::IsKeyDown(Key::A))
					movement += Right * (Speed * ts);

				if (Input::IsKeyDown(Key::W))
					movement += Direction * (Speed * ts);
				if (Input::IsKeyDown(Key::S))
					movement -= Direction * (Speed * ts);
				if (Input::IsKeyDown(Key::LEFT_SHIFT))
					movement -= Up * (Speed * ts);
				if (Input::IsKeyDown(Key::SPACE))
					movement += Up * (Speed * ts);

				Translation += Vector3(movement);
			}

			if (firstMouse)
			{
				mouseLastX = x;
				mouseLastY = y;
				firstMouse = false;
			}

			float diffx = x - mouseLastX;
			float diffy = mouseLastY - y;
			mouseLastX = x;
			mouseLastY = y;

			const float sensitivity = 0.1f;
			diffx *= sensitivity;
			diffy *= sensitivity;

			TargetYaw += diffx;
			TargetPitch += diffy;

		}	

		if (TargetPitch > 89.0f)
			TargetPitch = 89.0f;

		if (TargetPitch < -89.0f)
			TargetPitch = -89.0f;

		Direction.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
		Direction.y = sin(glm::radians(Pitch));
		Direction.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));

		SetDirection(glm::normalize(Direction));
		Right = glm::normalize(glm::cross(Up, Direction));
		
		
		if (controlled)
		{
			if (!wasControlled)
			{
				Input::HideMouse();
				wasControlled = true;
			}

			if (Input::IsMouseButtonDown(2))
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
		}
		else
		{
			if (wasControlled)
			{
				Input::ShowMouse();
				wasControlled = false;
			}
		}

		SetDirection(glm::normalize(Direction));

		
		mouseLastX = x;
		mouseLastY = y;

		return controlled;
	}

	Ref<EditorCamera> EditorCamera::Copy()
	{
		Ref<EditorCamera> copy = CreateRef<EditorCamera>();
		copy->Translation = this->Translation;
		copy->Yaw = this->Yaw;
		copy->Pitch = this->Pitch;
		copy->TargetYaw = copy->Yaw;
		copy->TargetPitch = copy->Pitch;

		return copy;
	}

	void EditorCamera::UpdateDirection()
	{
		Direction.x = cos(glm::radians(TargetYaw)) * cos(glm::radians(TargetPitch));
		Direction.y = sin(glm::radians(TargetPitch));
		Direction.z = sin(glm::radians(TargetYaw)) * cos(glm::radians(TargetPitch));
		Direction = glm::normalize(Direction);
		SetDirection(Direction);
	}

	void EditorCamera::SetYaw(float yaw)
	{
		TargetYaw = yaw;
		Yaw = yaw;
		UpdateDirection();
	}

	void EditorCamera::SetPitch(float pitch)
	{
		TargetPitch = pitch;
		Pitch = pitch;
		UpdateDirection();
	}

	json EditorCamera::Serialize()
	{
		BEGIN_SERIALIZE();

		SERIALIZE_VEC3(Translation);
		j["Yaw"] = glm::clamp(Yaw, 0.f, 360.f);
		j["Pitch"] = glm::clamp(Pitch, -90.f, 90.f);
		SERIALIZE_VAL(Speed);
		return j;
	}

	bool EditorCamera::Deserialize(const json& j)
	{
		DESERIALIZE_VEC3(j["Translation"], Translation);
		SetYaw(j["Yaw"]);
		SetPitch(j["Pitch"]);
		UpdateDirection();
		DESERIALIZE_VAL(Speed);

		return true;
	}
}

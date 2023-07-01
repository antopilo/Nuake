#pragma once
#include "../Core/Timestep.h"
#include "../Rendering/Camera.h"
#include "src/Core/Core.h"

namespace Nuake
{
	class EditorCamera : public Camera
	{
	public:
		EditorCamera()
		{
			Translation = Vector3(10, 10, 10);
			SetDirection(Vector3(-1, -1, -1));
		}

		void Update(Timestep ts, const bool hover);

		Vector3 TargetPos = Vector3(0, 0, 0);
		bool IsMoving = false;
		Ref<EditorCamera> Copy();

		json Serialize();
		bool Deserialize(const std::string& str);

		void SetYaw(float yaw);
		void SetPitch(float pitch);

	private:
		bool controlled = false;
		bool firstMouse = false;

		float mouseLastX;
		float mouseLastY;

		float Yaw = -135.f;
		float Pitch = -45.f;
		void UpdateDirection();
	};
}

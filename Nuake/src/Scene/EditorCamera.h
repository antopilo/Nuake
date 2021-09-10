#pragma once
#include "../Core/Timestep.h"
#include "../Rendering/Camera.h"

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

		void Update(Timestep ts);
	private:
		bool controlled = false;
		bool firstMouse = false;

		float mouseLastX;
		float mouseLastY;

		float Yaw = 0.f;
		float Pitch = 0.f;
	};
}

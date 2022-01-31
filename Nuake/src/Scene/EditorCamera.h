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

		void Update(Timestep ts);


		Ref<EditorCamera> Copy();
	private:
		bool controlled = false;
		bool firstMouse = false;

		float mouseLastX;
		float mouseLastY;

		float Yaw = -135.f;
		float Pitch = -45.f;
	};
}

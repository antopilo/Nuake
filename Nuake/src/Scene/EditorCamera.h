#pragma once
#include "../Core/Timestep.h"
#include "../Rendering/Camera.h"

namespace Nuake
{
	class EditorCamera : public Camera
	{
	public:
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

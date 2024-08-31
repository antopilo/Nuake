#pragma once
#include "../Core/Timestep.h"
#include "../Rendering/Camera.h"
#include "src/Core/Core.h"

namespace Nuake
{
	class EditorCamera : public Camera
	{
	private:
		bool m_IsFlying = false;

	public:
		
		EditorCamera()
		{
			SetYaw(-90.0f - 45.0f);
			SetPitch(-45.0f);
			Translation = Vector3(2, 2, 2);
		}

		void Update(Timestep ts, const bool hover);

		Vector3 TargetPos = Vector3(0, 0, 0);
		bool IsMoving = false;
		Ref<EditorCamera> Copy();

		json Serialize();
		bool Deserialize(const json& j);

		void SetYaw(float yaw);
		void SetPitch(float pitch);

		bool IsFlying() const { return m_IsFlying; }
	private:
		bool controlled = false;
		bool firstMouse = false;

		float mouseLastX;
		float mouseLastY;

		float Yaw = 0.0f;
		float Pitch = 0.0f;
		float TargetYaw = 0.f;
		float TargetPitch = 0.0f;
		void UpdateDirection();
	};
}

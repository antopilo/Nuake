#pragma once
#include "src/Core/Timestep.h"
#include "src/Core/Maths.h"
#include "src/Resource/Serializable.h"
#include "Frustum.h"
#include <src/Rendering/AABB.h>

namespace Nuake
{
	enum CAMERA_TYPE 
	{
		ORTHO,
		PERSPECTIVE
	};

	class EditorCamera;

	class Camera : public ISerializable
	{
	private:
		CAMERA_TYPE m_Type;
		Frustum m_Frustum;

		Vector3 Rotation = { 0.0f, 0.0f, 0.0f };
		Vector3 Scale = { 1.0f, 1.0f, 1.0f };
		Matrix4 m_Perspective;

	public:
		float AspectRatio = 16.0f / 9.0f;
		// TODO: remove duplicate direction and have a proper api.
		Vector3 up = Vector3(0.0f, 1.0f, 0.0f);
		Vector3 cameraFront = Vector3(0.0f, 0.0f, 1.0f);

		Vector3 cameraTarget;
		Vector3 cameraDirection;

		Vector3 Translation = { 0.0f, 0.0f, 0.0f };
		Vector3 cameraRight;
		Vector3 cameraUp;
		float Fov = 88.0f;
		float Exposure = 1.0f;
		float Speed = 1.0f;

		Camera();
		Camera(CAMERA_TYPE type, Vector3 position);

		void SetType(CAMERA_TYPE type);
		void OnWindowResize(int x, int y);

		void SetDirection(Vector3 direction);

		Vector3 GetTranslation();
		Vector3 GetDirection();
		Matrix4 GetPerspective();
		Matrix4 GetTransform();
		Matrix4 GetTransformRotation();
		
		bool BoxFrustumCheck(const AABB& aabb);

		json Serialize() override;
		bool Deserialize(const std::string& str) override;

		friend EditorCamera;
	};
}


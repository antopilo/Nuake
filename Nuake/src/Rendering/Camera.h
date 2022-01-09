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
		Vector3 Up = Vector3(0, 1, 0);
		Vector3 Right = Vector3(1, 0, 0);
		Matrix4 m_Perspective;


	public:
		float AspectRatio = 16.0f / 9.0f;
		
		Vector3 Direction = Vector3(0, 0, 1);
		Vector3 Translation = { 0.0f, 0.0f, 0.0f };
		float Fov = 88.0f;
		float Exposure = 1.0f;
		float Gamma = 2.2f;
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
		inline Vector3 GetRight() const { return Right; }
		inline Vector3 GetUp() const { return glm::cross(Direction, Right); }
		bool BoxFrustumCheck(const AABB& aabb);
		Frustum GetFrustum();
		json Serialize() override;
		bool Deserialize(const std::string& str) override;

		friend EditorCamera;
	};
}


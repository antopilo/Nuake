#include "Camera.h"
#include "src/Core/Maths.h"
#include "../Core/Input.h"
#include <glm/ext/vector_float3.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_float4x4.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <GLFW/glfw3.h>


namespace Nuake
{
	Camera::Camera(CAMERA_TYPE type, Vector3 position)
	{
		m_Type = PERSPECTIVE;
		Translation = position;

		Direction = Vector3(0, 0, 1);
		Right = glm::normalize(glm::cross(Vector3(0, 1, 0), Direction));
		Up = glm::cross(Direction, Right);

		m_Frustum = Frustum(GetTransform());
	}

	Camera::Camera() 
	{
		m_Type = PERSPECTIVE;
		Translation = Vector3(0, 0, 0);
		Direction = Vector3(0, 0, 1);
		Right = glm::normalize(glm::cross(Vector3(0, 1, 0), Direction));
		Up = glm::cross(Direction, Right);
	}

	void Camera::SetType(CAMERA_TYPE type)
	{
		m_Type = type;
	}

	void Camera::OnWindowResize(float x, float y)
	{
		AspectRatio = (float)x / (float)y;
		float width = (float)y * (16.0f * 9.0f);
		float height = (float)y;
	}

	void Camera::SetDirection(Vector3 direction)
	{
		Direction = glm::normalize(direction);
		Right = glm::normalize(glm::cross(Vector3(0, 1, 0), Direction));
		m_View = lookAt(Translation, Translation + glm::normalize(Direction), Vector3(0, 1, 0));
	}

	void Camera::SetDirection(const Quat& direction)
	{
		// TODO: Calculate forward and Right from quaternion.
		assert("Not implemented!");
	}

	Vector3 Camera::GetTranslation() {
		return Translation;
	}

	Vector3 Camera::GetDirection() {
		return Direction;
	}

	Matrix4 Camera::GetPerspective()
	{
		//TODO: Add perspective options
		m_Perspective = glm::perspectiveFov(glm::radians(Fov), 9.0f * AspectRatio, 9.0f, 0.1f, 1000.0f);
		return m_Perspective;
	}

	void Camera::SetTransform(const Matrix4& transform)
	{
		m_View = transform;
	}

	Matrix4 Camera::GetTransform()
	{
		return m_View;
	}

	Matrix4 Camera::GetTransformRotation()
	{
		return lookAt(Vector3(), Direction, Up);
	}

	bool Camera::BoxFrustumCheck(const AABB& aabb)
	{
		m_Frustum = Frustum(GetPerspective() * GetTransform());
		return m_Frustum.IsBoxVisible(aabb.Min, aabb.Max);
	}

	Frustum Camera::GetFrustum()
	{
		m_Frustum = Frustum(GetPerspective() * GetTransform());
		return m_Frustum;
	}

	json Camera::Serialize()
	{
		BEGIN_SERIALIZE();
		SERIALIZE_VAL(m_Type);
		SERIALIZE_VAL(AspectRatio);
		SERIALIZE_VEC3(Translation)
		SERIALIZE_VEC3(Direction)
		SERIALIZE_VAL(Fov);
		SERIALIZE_VAL(Exposure);
		SERIALIZE_VAL(Speed);
		END_SERIALIZE();
	}

	bool Camera::Deserialize(const std::string& str)
	{
		BEGIN_DESERIALIZE();
		j = j["CameraInstance"];
		this->m_Type = (CAMERA_TYPE)j["m_Type"];
		if(j.contains("Translation"))
			DESERIALIZE_VEC3(j["Translation"], Translation);

		//if (j.contains("Direction"))
		//	DESERIALIZE_VEC3(j["Direction"], Direction);

		this->AspectRatio = j["AspectRatio"];
		this->Fov = j["Fov"];
		this->Exposure = j["Exposure"];
		this->Speed = j["Speed"];
		return false;
	}
}

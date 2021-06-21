
#include "Camera.h"
#include "../Core/Input.h"
#include <glm\ext\vector_float3.hpp>
#include <glm\ext\matrix_clip_space.hpp>
#include <glm\ext\matrix_float4x4.hpp>
#include <glm\ext\matrix_transform.hpp>
#include <GLFW\glfw3.h>

Camera::Camera(CAMERA_TYPE type, glm::vec3 position) {
	m_Type = PERSPECTIVE;
	Translation = position;
	cameraDirection = glm::vec3(0, 0, 1);
	up = glm::vec3(0.0f, 1.0f, 0.0f);
	cameraRight = glm::normalize(glm::cross(up, cameraDirection));
	cameraUp = glm::cross(cameraDirection, cameraRight);
	cameraFront = cameraDirection;
}

Camera::Camera() {
	m_Type = PERSPECTIVE;
	Translation = glm::vec3(0, 0, 0);
	cameraDirection = glm::vec3(0, 0, 1);
	up = glm::vec3(0.0f, 1.0f, 0.0f);
	cameraRight = glm::normalize(glm::cross(up, cameraDirection));
	cameraUp = glm::cross(cameraDirection, cameraRight);
}

void Camera::SetType(CAMERA_TYPE type)
{
	m_Type = type;
}

void Camera::OnWindowResize(int x, int y)
{
	AspectRatio = (float)x / (float)y;
	float width = y * (16 * 9);
	float height = y;
}

void Camera::SetDirection(Vector3 direction)
{
	//cam->cameraDirection.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
	//cam->cameraDirection.y = sin(glm::radians(Pitch));
	//cam->cameraDirection.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
	//cam->cameraFront = glm::normalize(cam->cameraDirection);
	//cam->cameraRight = glm::normalize(glm::cross(cam->up, cam->cameraFront));
	cameraDirection = glm::normalize(direction);
	cameraFront = cameraDirection;
	cameraRight = glm::normalize(glm::cross(up, cameraFront));
}

glm::vec3 Camera::GetTranslation() {
	return Translation;
}

glm::vec3 Camera::GetDirection() {
	return cameraDirection;
}

glm::mat4 Camera::GetPerspective()
{
	//TODO: Add perspective options
	m_Perspective = glm::perspectiveFov(glm::radians(Fov), 9.0f * AspectRatio, 9.0f, 0.1f, 1000.0f);
	//m_Perspective = glm::ortho(-8.0f, 8.0f, -4.5f, 4.5f, -1.0f, 1.0f);
	return m_Perspective;
}

glm::mat4 Camera::GetTransform()
{
	glm::mat4 tr = lookAt(Translation, Translation + cameraFront, cameraUp);
	return tr;

}

glm::mat4 Camera::GetTransformRotation()
{
	return lookAt(glm::vec3(), cameraFront, cameraUp);
}

json Camera::Serialize()
{
	BEGIN_SERIALIZE();
	SERIALIZE_VAL(m_Type);
	SERIALIZE_VAL(AspectRatio);
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
	this->AspectRatio = j["AspectRatio"];
	this->Fov = j["Fov"];
	this->Exposure = j["Exposure"];
	this->Speed = j["Speed"];
	return false;
}




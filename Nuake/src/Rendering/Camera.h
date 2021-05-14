#pragma once
#include "../Core/Timestep.h"
#include <glm\vec3.hpp>
#include <glm\ext\matrix_float4x4.hpp>
enum CAMERA_TYPE {
	ORTHO,
	PERSPECTIVE
};
class EditorCamera;
class __declspec(dllexport) Camera
{
private:
	CAMERA_TYPE m_Type;

	float AspectRatio = 16.0f / 9.0f;

	
	glm::vec3 Rotation = { 0.0f, 0.0f, 0.0f };
	glm::vec3 Scale = { 1.0f, 1.0f, 1.0f };

	glm::mat4 m_Perspective;


	bool controlled = true;
	bool firstMouse = true;

	float Yaw = 0;
	float Pitch = 0;
	float mouseLastX = 0;
	float mouseLastY = 0;

public:
	glm::vec3 up; // = glm::vec3(0.0f, 1.0f, 0.0f);
	glm::vec3 cameraFront; // = glm::vec3(0.0f, 0.0f, 1.0f);

	glm::vec3 cameraTarget; // = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec3 cameraDirection; // = glm::normalize(Translation - cameraTarget);

	glm::vec3 Translation = { 0.0f, 0.0f, 0.0f };
	glm::vec3 cameraRight; // = glm::normalize(glm::cross(up, cameraDirection));
	glm::vec3 cameraUp; // = glm::cross(cameraDirection, cameraRight);
	float Fov = 88.0f;
	float Exposure = 1.0f;
	float Speed = 1.0f;

	Camera();
	Camera(CAMERA_TYPE type, glm::vec3 position);

	void SetType(CAMERA_TYPE type);
	void OnWindowResize(int x, int y);


	glm::vec3 GetTranslation();
	glm::vec3 GetDirection();
	glm::mat4 GetPerspective();
	glm::mat4 GetTransform();
	glm::mat4 GetTransformRotation();

	friend EditorCamera;
};

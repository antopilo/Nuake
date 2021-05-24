#pragma once
#include "../Core/Timestep.h"
#include "../Core/Maths.h"
#include "../Resource/Serializable.h"

enum CAMERA_TYPE {
	ORTHO,
	PERSPECTIVE
};

class EditorCamera;

// TODO: Remove logic from here.
class Camera : public ISerializable
{
private:
	CAMERA_TYPE m_Type;

	float AspectRatio = 16.0f / 9.0f;
	Vector3 Rotation = { 0.0f, 0.0f, 0.0f };
	Vector3 Scale = { 1.0f, 1.0f, 1.0f };
	Matrix4 m_Perspective;

public:
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

	Vector3 GetTranslation();
	Vector3 GetDirection();
	Matrix4 GetPerspective();
	Matrix4 GetTransform();
	Matrix4 GetTransformRotation();

	json Serialize() override;
	bool Deserialize(const std::string& str) override;

	friend EditorCamera;
};

#pragma once
#include "../Core/Timestep.h"
#include "../Rendering/Camera.h"

class EditorCamera : public Camera
{
private:
	bool controlled = false;
	bool firstMouse = false;

	float mouseLastX;
	float mouseLastY;

	float Yaw = 0.f;
	float Pitch = 0.f;
public:
	void Update(Timestep ts);
};
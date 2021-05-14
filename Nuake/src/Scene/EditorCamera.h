#pragma once
#include "../Core/Timestep.h"
#include "../Rendering/Camera.h"
class EditorCamera : public Camera
{
public:
	void Update(Timestep ts);
};
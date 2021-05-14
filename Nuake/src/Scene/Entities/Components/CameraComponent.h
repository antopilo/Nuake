#pragma once
#include "TransformComponent.h"
#include "../Core/Core.h"
class __declspec(dllexport) CameraComponent {
public:
	Ref<Camera> CameraInstance;
	TransformComponent* transformComponent;

	CameraComponent();

	void DrawEditor();
};
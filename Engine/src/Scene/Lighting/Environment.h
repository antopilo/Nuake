#pragma once
#include <glm\ext\vector_float4.hpp>
#include <glm\ext\vector_float3.hpp>
#include "../Core/Core.h"
#include "../Rendering/ProceduralSky.h"

class __declspec(dllexport) Environment
{
public:
	Environment();

	float VolumetricFog = 0.95f;
	float VolumetricStepCount = 100.f;
	glm::vec4 AmbientColor;
	Ref<ProceduralSky> ProceduralSkybox;

	glm::vec4 m_AmbientColor;

	glm::vec4 GetAmbientColor();

	void SetAmbientColor(glm::vec4 color);

	void Push();
};

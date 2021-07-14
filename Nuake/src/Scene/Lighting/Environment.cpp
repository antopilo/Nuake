#pragma once
#include "Environment.h"
#include "src/Rendering/Renderer.h"
#include "src/Core/Core.h"

namespace Nuake {
	Environment::Environment()
	{
		m_AmbientColor = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
		ProceduralSkybox = CreateRef<ProceduralSky>();
	}

	// Ambient Light
	glm::vec4 Environment::GetAmbientColor()
	{
		return m_AmbientColor;
	}

	void Environment::SetAmbientColor(glm::vec4 color)
	{
		m_AmbientColor = color;
	}

	void Environment::Push() 
	{
		//Renderer::m_Shader->SetUniform3f("u_AmbientColor", m_AmbientColor.r, m_AmbientColor.g, m_AmbientColor.b);
		//Renderer::m_Shader->SetUniform4f("u_AmbientColor", 1.0f, 1.0f, 1.0f, 1.0f);
		Renderer::m_Shader->SetUniform1f("u_FogAmount", VolumetricFog);
		Renderer::m_Shader->SetUniform1f("u_FogStepCount", VolumetricStepCount);
	}

	json Environment::Serialize()
	{
		BEGIN_SERIALIZE();
		SERIALIZE_VAL(VolumetricFog);
		SERIALIZE_VAL(VolumetricStepCount);
		SERIALIZE_VEC4(AmbientColor);
		SERIALIZE_OBJECT(ProceduralSkybox);
		END_SERIALIZE();
	}

	bool Environment::Deserialize(const std::string& str)
	{
		return false;
	}
}


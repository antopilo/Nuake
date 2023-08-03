#pragma once
#include "Environment.h"
#include "src/Rendering/Renderer.h"
#include "src/Core/Core.h"

namespace Nuake {
	Environment::Environment()
	{
		m_AmbientColor = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
		ProceduralSkybox = CreateRef<ProceduralSky>();
		mBloom = CreateScope<Bloom>(4);
		mVolumetric = CreateScope<Volumetric>();
		mSSAO = CreateScope<SSAO>();
	}

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
		Renderer::m_Shader->SetUniform1f("u_FogAmount", VolumetricFog);
		Renderer::m_Shader->SetUniform1f("u_FogStepCount", VolumetricStepCount);
	}

	Ref<Environment> Environment::Copy()
	{
		Ref<Environment> copy = CreateRef<Environment>();
		copy->CurrentSkyType = this->CurrentSkyType;
		copy->ProceduralSkybox = this->ProceduralSkybox->Copy();
		copy->VolumetricEnabled = this->VolumetricEnabled;
		copy->VolumetricFog = this->VolumetricFog;
		copy->VolumetricStepCount = this->VolumetricStepCount;

		copy->Exposure = this->Exposure;
		copy->Gamma = this->Gamma;

		copy->BloomEnabled = this->BloomEnabled;
		copy->ClearColor = this->ClearColor;
		copy->AmbientColor = this->AmbientColor;

		return copy;
	}

	json Environment::Serialize()
	{
		BEGIN_SERIALIZE();
		SERIALIZE_VAL(CurrentSkyType);
		SERIALIZE_VAL(VolumetricFog);
		SERIALIZE_VAL(VolumetricStepCount);
		SERIALIZE_VEC4(AmbientColor);
		SERIALIZE_OBJECT(ProceduralSkybox);
		END_SERIALIZE();
	}

	bool Environment::Deserialize(const json& j)
	{
		if (j.contains("CurrentSkyType"))
			CurrentSkyType = j["CurrentSkyType"];
		if (j.contains("VolumetricFog"))
			VolumetricFog = j["VolumetricFog"];
		if (j.contains("VolumetricStepCount"))
			VolumetricStepCount = j["VolumetricStepCount"];
		if (j.contains("ProceduralSkybox"))
		{
			ProceduralSkybox->Deserialize(j["ProceduralSkybox"]);
		}
			
		return false;
	}
}


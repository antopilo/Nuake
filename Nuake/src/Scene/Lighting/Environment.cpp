#pragma once
#include "Environment.h"

#include "src/Core/Core.h"


namespace Nuake 
{
	Environment::Environment() :
		m_AmbientColor(1.0f, 1.0f, 1.0f, 1.0f)
	{
	
		ProceduralSkybox = CreateRef<ProceduralSky>();

		CurrentSkyType = SkyType::ProceduralSky;
		AmbientColor = Color(0.2, 0.2, 0.2, 1.0f);
		AmbientTerm = 0.5f;

		VolumetricEnabled = true;
		VolumetricFog = 0.9f;
		VolumetricStepCount = 50.0f;

		// Bloom
		BloomEnabled = true;
		SSAOEnabled = true;
		SSREnabled = true;

		// DOF
		DOFEnabled = false;
		BarrelDistortionEnabled = false;
		BarrelDistortion = 0.0f;
		BarrelEdgeDistortion = 0.0f;
		BarrelScale = 1.0f;

		// Vignette
		VignetteEnabled = false;
		VignetteIntensity = 150.0f;
		VignetteExtend = 0.150f;

		// POST FX
		mBloom = CreateScope<Bloom>(4);
		mVolumetric = CreateScope<Volumetric>();
		mSSAO = CreateScope<SSAO>();
		mSSR = CreateScope<SSR>();
	}

	Environment::Environment(const std::string& path) : Environment()
	{
		this->Path = path;
	}

	glm::vec4 Environment::GetAmbientColor()
	{
		return m_AmbientColor;
	}

	void Environment::SetAmbientColor(glm::vec4 color)
	{
		m_AmbientColor = color;
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
		j["UUID"] = static_cast<uint64_t>(ID);

		// Bloom
		SERIALIZE_VAL(BloomEnabled);
		SERIALIZE_VAL_LBL("BloomThreshold", mBloom->GetThreshold());
		SERIALIZE_VAL_LBL("BloomIteration", mBloom->GetIteration());

		// Volumetric
		SERIALIZE_VAL(VolumetricEnabled);
		SERIALIZE_VAL(VolumetricFog);
		SERIALIZE_VAL(VolumetricStepCount);

		// SSR
		SERIALIZE_VAL(SSREnabled);
		SERIALIZE_VAL_LBL("SSRRayStep", mSSR->RayStep);
		SERIALIZE_VAL_LBL("SSRIterationCount", mSSR->IterationCount);
		SERIALIZE_VAL_LBL("SSRDistanceBias", mSSR->DistanceBias);
		SERIALIZE_VAL_LBL("SSRSampleCount", mSSR->SampleCount);
		SERIALIZE_VAL_LBL("SSRSamplingEnabled", mSSR->SamplingEnabled);
		SERIALIZE_VAL_LBL("SSRExpoStep", mSSR->ExpoStep);
		SERIALIZE_VAL_LBL("SSRAdaptiveStep", mSSR->AdaptiveStep);
		SERIALIZE_VAL_LBL("SSRBinarySearch", mSSR->BinarySearch);
		SERIALIZE_VAL_LBL("SSRSampleingCoefficient", mSSR->SampleingCoefficient);

		// SSAO
		SERIALIZE_VAL(SSAOEnabled);
		SERIALIZE_VAL_LBL("SSAORadius", mSSAO->Radius);
		SERIALIZE_VAL_LBL("SSAOBias", mSSAO->Bias);
		SERIALIZE_VAL_LBL("SSAOArea", mSSAO->Area);
		SERIALIZE_VAL_LBL("SSAOFalloff", mSSAO->Falloff);
		SERIALIZE_VAL_LBL("SSAOStrength", mSSAO->Strength);

		// DOF
		SERIALIZE_VAL(DOFEnabled);

		// Barrel distortion
		SERIALIZE_VAL(BarrelDistortionEnabled);
		SERIALIZE_VAL(BarrelDistortion);
		SERIALIZE_VAL(BarrelEdgeDistortion);
		SERIALIZE_VAL(BarrelScale);

		// Vignette
		SERIALIZE_VAL(VignetteEnabled);
		SERIALIZE_VAL(VignetteExtend);
		SERIALIZE_VAL(VignetteIntensity);

		SERIALIZE_VAL(AmbientTerm);
		SERIALIZE_VEC4(AmbientColor);

		SERIALIZE_OBJECT(ProceduralSkybox);
		END_SERIALIZE();
	}

	bool Environment::Deserialize(const json& j)
	{
		// SKY
		if (j.contains("CurrentSkyType"))
		{
			CurrentSkyType = j["CurrentSkyType"];
		}

		if (j.contains("ProceduralSkybox"))
		{
			ProceduralSkybox->Deserialize(j["ProceduralSkybox"]);
		}

		// Ambient
		if (j.contains("AmbientTerm"))
		{
			AmbientTerm = j["AmbientTerm"];
		}

		if (j.contains("AmbientColor"))
		{
			DESERIALIZE_VEC4(j["AmbientColor"], AmbientColor);
		}

		// Bloom
		if (j.contains("BloomEnabled"))
		{
			BloomEnabled = j["BloomEnabled"];
		}
		
		if (j.contains("BloomThreshold"))
		{
			mBloom->SetThreshold(j["BloomThreshold"]);
		}

		if (j.contains("BloomIteration"))
		{
			mBloom->SetIteration(j["BloomIteration"]);
		}

		// SSR
		if (j.contains("SSREnabled"))
		{
			SSREnabled = j["SSREnabled"];
		}

		if (j.contains("SSRRayStep"))
		{
			mSSR->RayStep = j["SSRRayStep"];
		}

		if (j.contains("SSRIterationCount"))
		{
			mSSR->IterationCount = j["SSRIterationCount"];
		}

		if (j.contains("SSRDistanceBias"))
		{
			mSSR->DistanceBias = j["SSRDistanceBias"];
		}

		if (j.contains("SSRSampleCount"))
		{
			mSSR->SampleCount = j["SSRSampleCount"];
		}

		if (j.contains("SSRSamplingEnabled"))
		{
			mSSR->SamplingEnabled = j["SSRSamplingEnabled"];
		}

		if (j.contains("SSRExpoStep"))
		{
			mSSR->ExpoStep = j["SSRExpoStep"];
		}

		if (j.contains("SSRAdaptiveStep"))
		{
			mSSR->AdaptiveStep = j["SSRAdaptiveStep"];
		}

		if (j.contains("SSRBinarySearch"))
		{
			mSSR->BinarySearch = j["SSRBinarySearch"];
		}

		if (j.contains("SSRSampleingCoefficient"))
		{
			mSSR->SampleingCoefficient = j["SSRSampleingCoefficient"];
		}

		// Volumetric
		if (j.contains("VolumetricEnabled"))
		{
			VolumetricEnabled = j["VolumetricEnabled"];
		}

		if (j.contains("VolumetricFog"))
		{
			VolumetricFog = j["VolumetricFog"];
		}

		if (j.contains("VolumetricStepCount"))
		{
			VolumetricStepCount = j["VolumetricStepCount"];
		}
		
		// SSAO
		if (j.contains("SSAOEnabled"))
		{
			SSAOEnabled = j["SSAOEnabled"];
		}

		if (j.contains("SSAORadius"))
		{
			mSSAO->Radius = j["SSAORadius"];
		}

		if (j.contains("SSAOBias"))
		{
			mSSAO->Bias = j["SSAOBias"];
		}

		if (j.contains("SSAOArea"))
		{
			mSSAO->Area = j["SSAOArea"];
		}

		if (j.contains("SSAOFalloff"))
		{
			mSSAO->Falloff = j["SSAOFalloff"];
		}

		if (j.contains("SSAOStrength"))
		{
			mSSAO->Strength = j["SSAOStrength"];
		}

		// DOF
		if (j.contains("DOFEnabled"))
		{
			DOFEnabled = j["DOFEnabled"];
		}

		// Barrel distortion
		if (j.contains("BarrelDistortionEnabled"))
		{
			BarrelDistortionEnabled = j["BarrelDistortionEnabled"];
		}

		if (j.contains("BarrelDistortion"))
		{
			BarrelDistortion = j["BarrelDistortion"];
		}

		if (j.contains("BarrelEdgeDistortion"))
		{
			BarrelEdgeDistortion = j["BarrelEdgeDistortion"];
		}

		if (j.contains("BarrelScale"))
		{
			BarrelScale = j["BarrelScale"];
		}

		// Vignette
		if (j.contains("VignetteEnabled"))
		{
			VignetteEnabled = j["VignetteEnabled"];
		}

		if (j.contains("VignetteExtend"))
		{
			VignetteExtend = j["VignetteExtend"];
		}

		if (j.contains("VignetteIntensity"))
		{
			VignetteIntensity = j["VignetteIntensity"];
		}

		return false;
	}
}


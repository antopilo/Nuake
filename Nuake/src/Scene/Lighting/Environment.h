#pragma once
#include "src/Core/Maths.h"
#include "src/Core/Core.h"
#include "src/Scene/Environment/ProceduralSky.h"
#include "src/Resource/Resource.h"
#include "src/Resource/Serializable.h"

#include "src/Rendering/PostFX/Bloom.h"
#include "src/Rendering/PostFX/Volumetric.h"
#include "src/Rendering/PostFX/SSAO.h"
#include "src/Rendering/PostFX/SSR.h"

#include <vector>

namespace Nuake
{
	enum class SkyType 
	{
		ProceduralSky = 0,
		ClearColor = 1
		// CubeMap
	};

	class Environment : public Resource, ISerializable
	{
	public:
		Environment(const std::string& path);
		Environment();

		SkyType CurrentSkyType;
		Ref<ProceduralSky> ProceduralSkybox;

		Color AmbientColor;

		bool VolumetricEnabled;
		float VolumetricFog;
		float VolumetricStepCount;

		float AmbientTerm;
		float Exposure = 3.5f;
		float Gamma = 2.2f;

		bool BloomEnabled = true;
		Scope<Bloom> mBloom;
		Scope<Volumetric> mVolumetric;
		bool SSAOEnabled = true;
		Scope<SSAO> mSSAO;

		bool SSREnabled = true;
		Scope<SSR> mSSR;

		bool DOFEnabled = false;
		float DOFFocalDepth = 1.0f;
		float DOFFocalLength = 16.0f;
		float DOFFstop = 1.0f;
		bool DOFAutoFocus = false;
		bool DOFShowFocus = false;
		bool DOFManualFocus = false;
		int DOFSamples = 3;
		int DOFrings = 3;
		float DOFStart = 1.0f;
		float DOFDist = 3.0f;
		float DOFCoc = 0.03f;
		float DOFMaxBlue = 1.0f;
		float DOFThreshold = 1.0f;
		float DOFGain = 100.f;
		float DOFBias = 0.0f;
		float DOFFringe = 0.0f;
		float DOFNAmmount = 0.0001f;
		float DOFDbSize = 1.25f;
		float DOFFeather = 1.0f;

		bool BarrelDistortionEnabled = true;
		float BarrelDistortion = 0.f;
		float BarrelEdgeDistortion = 0.f;
		float BarrelScale = 1.0f;

		bool VignetteEnabled = true;
		float VignetteIntensity = 15.0f;
		float VignetteExtend = 0.5f;

		Vector3 ClearColor;

		glm::vec4 m_AmbientColor;
		glm::vec4 GetAmbientColor();
		void SetAmbientColor(glm::vec4 color);

		Ref<Environment> Copy();

		json Serialize() override;
		bool Deserialize(const json& j) override;
	};
}

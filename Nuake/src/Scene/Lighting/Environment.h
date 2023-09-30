#pragma once
#include "src/Core/Maths.h"
#include "src/Core/Core.h"
#include "src/Scene/Environment/ProceduralSky.h"
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

	class Environment : public ISerializable
	{
	public:
		Environment();

		SkyType CurrentSkyType;
		Ref<ProceduralSky> ProceduralSkybox;

		Color AmbientColor;

		bool VolumetricEnabled;
		float VolumetricFog;
		float VolumetricStepCount;

		float AmbientTerm;
		float Exposure = 3.5f;
		float Gamma = 1.1f;

		bool BloomEnabled = true;
		Scope<Bloom> mBloom;
		Scope<Volumetric> mVolumetric;
		bool SSAOEnabled = true;
		Scope<SSAO> mSSAO;

		bool SSREnabled = true;
		Scope<SSR> mSSR;

		bool DOFEnabled = false;

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

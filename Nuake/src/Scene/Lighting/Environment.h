#pragma once
#include "src/Core/Maths.h"
#include "src/Core/Core.h"
#include "src/Scene/Environment/ProceduralSky.h"
#include "src/Resource/Serializable.h"

#include "src/Rendering/PostFX/Bloom.h"
#include "src/Rendering/PostFX/Volumetric.h"
#include "src/Rendering/PostFX/SSAO.h"

#include <vector>

namespace Nuake
{
	enum class SkyType {
		ProceduralSky = 0,
		ClearColor = 1
		// CubeMap
	};

	class Environment : public ISerializable
	{
	public:
		Environment();

		SkyType CurrentSkyType = SkyType::ProceduralSky;

		Color AmbientColor = Color(0, 0, 0, 1);
		Ref<ProceduralSky> ProceduralSkybox;

		bool VolumetricEnabled = true;
		float VolumetricFog = 0.90f;
		float VolumetricStepCount = 50.f;

		float Exposure = 3.5f;
		float Gamma = 1.1f;

		bool BloomEnabled = false;
		Scope<Bloom> mBloom;
		Scope<Volumetric> mVolumetric;

		bool SSAOEnabled = true;
		Scope<SSAO> mSSAO;

		Vector3 ClearColor;

		glm::vec4 m_AmbientColor;
		glm::vec4 GetAmbientColor();
		void SetAmbientColor(glm::vec4 color);

		void Push();

		Ref<Environment> Copy();

		json Serialize() override;
		bool Deserialize(const json& j) override;
	};
}

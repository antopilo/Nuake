#pragma once
#include "src/Core/Maths.h"
#include "src/Core/Core.h"
#include "src/Scene/Environment/ProceduralSky.h"
#include "src/Resource/Serializable.h"
#include "src/Rendering/PostFX/Bloom.h"

#include <vector>

namespace Nuake
{
	class Environment : public ISerializable
	{
	public:
		Environment();
		float VolumetricFog = 0.90f;
		float VolumetricStepCount = 50.f;

		Scope<Bloom> mBloom;

		Vector3 ClearColor;

		glm::vec4 AmbientColor;
		Ref<ProceduralSky> ProceduralSkybox;

		glm::vec4 m_AmbientColor;
		glm::vec4 GetAmbientColor();
		void SetAmbientColor(glm::vec4 color);

		void Push();

		json Serialize() override;
		bool Deserialize(const std::string& str) override;
	};
}

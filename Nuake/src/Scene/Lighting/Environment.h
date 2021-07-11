#pragma once
#include "src/Core/Maths.h"
#include "src/Core/Core.h"
#include "src/Scene/Environment/ProceduralSky.h"
#include "src/Resource/Serializable.h"

namespace Nuake
{
	class Environment : public ISerializable
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

		json Serialize() override;
		bool Deserialize(const std::string& str) override;
	};
}

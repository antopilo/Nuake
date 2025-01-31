#pragma once
#include "Nuake/Core/Core.h"
#include "Nuake/Core/Maths.h"

#include "Nuake/Rendering/Vertex.h"
#include "Nuake/Resource/Serializable.h"

namespace Nuake
{
	class Camera;
	class ProceduralSky : ISerializable {
	public:
		float SurfaceRadius = 6360e3f;
		float AtmosphereRadius = 6380e3f;
		Vector3 RayleighScattering = Vector3(58e-7f, 135e-7f, 331e-7f);
		Vector3 MieScattering = Vector3(2e-5f);
		float SunIntensity = 2.0;

		Vector3 CenterPoint = Vector3(0.f, -SurfaceRadius, 0.f);
		Vector3 SunDirection = Vector3(0.20000f, 0.95917f, 0.20000f);
		unsigned int VAO;
		unsigned int VBO;
		ProceduralSky();
		void Draw(Matrix4 projection, Matrix4 view);

		Vector3 GetSunDirection();

		Ref<ProceduralSky> Copy();

		json Serialize() override;
		bool Deserialize(const json& j) override;
	};
}

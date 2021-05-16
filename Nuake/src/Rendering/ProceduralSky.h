#pragma once
#include <glm/ext/vector_float3.hpp>
#include "Vertex.h"
#include "../Core/Core.h"
#include "../Resource/Serializable.h"

class Camera;
class ProceduralSky : ISerializable {
public:
	float SurfaceRadius = 6360e3;
	float AtmosphereRadius = 6380e3;
	glm::vec3 RayleighScattering = glm::vec3(58e-7, 135e-7, 331e-7);
	glm::vec3 MieScattering = glm::vec3(2e-5);
	float SunIntensity = 100.0;

	glm::vec3 CenterPoint = glm::vec3(0, -SurfaceRadius, 0);
	glm::vec3 SunDirection = glm::vec3(0.20000, 0.95917, 0.20000);
	unsigned int VAO;
	unsigned int VBO;
	ProceduralSky();
	void Draw(Ref<Camera> cam);

	glm::vec3 GetSunDirection();

	json Serialize() override;
	bool Deserialize(const std::string& str) override;
};
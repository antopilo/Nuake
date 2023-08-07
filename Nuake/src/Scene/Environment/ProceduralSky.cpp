#include "ProceduralSky.h"

#include "src/Rendering/Renderer.h"
#include "src/Rendering/Shaders/Shader.h"
#include "src/Rendering/Camera.h"

#include <GL/glew.h>

namespace Nuake
{
	// TODO: move this to primitive
	ProceduralSky::ProceduralSky() {
		float quadVertices[] = {
			// positions        // texture Coords
			-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
			 1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
			-1.0f,  1.0f, 0.0f, 0.0f, 1.0f,


			 1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
			-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
			 1.0f,  1.0f, 0.0f, 1.0f, 1.0f
		};

		// setup plane VAO
		glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &VBO);
		glBindVertexArray(VAO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	}

	void ProceduralSky::Draw(Matrix4 projection, Matrix4 view) {
		Shader* skyShader = ShaderManager::GetShader("resources/Shaders/atmospheric_sky.shader");
		skyShader->Bind();
		skyShader->SetUniform1f("SurfaceRadius", SurfaceRadius);
		skyShader->SetUniform1f("AtmosphereRadius", AtmosphereRadius);
		skyShader->SetUniform1f("SunIntensity", SunIntensity);

		skyShader->SetUniform3f("RayleighScattering",
			RayleighScattering.r,
			RayleighScattering.g,
			RayleighScattering.b);

		skyShader->SetUniform3f("MieScattering",
			MieScattering.r,
			MieScattering.g,
			MieScattering.b);

		skyShader->SetUniform3f("CenterPoint",
			CenterPoint.x,
			CenterPoint.y,
			CenterPoint.z);

		skyShader->SetUniform3f("SunDirection",
			SunDirection.x,
			SunDirection.y,
			SunDirection.z);

		skyShader->SetUniformMat4f("Projection", projection);
		skyShader->SetUniformMat4f("View", view);
		//glm::vec3 CamRight = cam->cameraRight;
		//Renderer::m_ProceduralSkyShader->SetUniform3f("CamRight",
		//	CamRight.x,
		//	CamRight.y,
		//	CamRight.z);
		//
		//glm::vec3 CamUp = cam->cameraUp;//glm::normalize(glm::cross(CameraDirection, CamRight));

		//Renderer::m_ProceduralSkyShader->SetUniform3f("CamUp",
		//	CamUp.x,
		//	CamUp.y,
		//	CamUp.z);


		glBindVertexArray(VAO);
		glDrawArrays(GL_TRIANGLES, 0, 6);
	}

	glm::vec3 ProceduralSky::GetSunDirection() {
		return SunDirection;
	}

	Ref<ProceduralSky> ProceduralSky::Copy()
	{
		auto copy = CreateRef<ProceduralSky>();
		copy->SurfaceRadius = this->SurfaceRadius;
		copy->AtmosphereRadius = this->AtmosphereRadius;
		copy->RayleighScattering = this->RayleighScattering;
		copy->MieScattering = this->MieScattering;
		copy->SunIntensity = this->SunIntensity;
		copy->CenterPoint = this->CenterPoint;
		copy->SunDirection = this->SunDirection;
		
		return copy;
	}

	json ProceduralSky::Serialize()
	{
		BEGIN_SERIALIZE()
		SERIALIZE_VAL(SurfaceRadius);
		SERIALIZE_VAL(AtmosphereRadius);
		SERIALIZE_VEC3(RayleighScattering);
		SERIALIZE_VEC3(MieScattering);
		SERIALIZE_VAL(SunIntensity);
		SERIALIZE_VEC3(SunDirection);
		SERIALIZE_VEC3(CenterPoint);
		END_SERIALIZE();
	}

	bool ProceduralSky::Deserialize(const json& j)
	{
		DESERIALIZE_VEC3(j["SunDirection"], SunDirection)
		DESERIALIZE_VEC3(j["RayleighScattering"], RayleighScattering)
		DESERIALIZE_VEC3(j["MieScattering"], MieScattering)
		if (j.contains("CenterPoint"))
		{
			DESERIALIZE_VEC3(j["CenterPoint"], CenterPoint)
		}
		SurfaceRadius = j["SurfaceRadius"];
		AtmosphereRadius = j["AtmosphereRadius"];
		SunIntensity = j["SunIntensity"];
		return true;
	}
}

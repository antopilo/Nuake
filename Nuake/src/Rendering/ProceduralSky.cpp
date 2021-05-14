#include "ProceduralSky.h"
#include "Renderer.h"
#include "Shaders/Shader.h"
#include "Camera.h"
#include <GL/glew.h>

ProceduralSky::ProceduralSky() {
	float quadVertices[] = {
		// positions        // texture Coords
		-1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
		-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
		 1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
		 1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
		 1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
		 -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
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

void ProceduralSky::Draw(Ref<Camera> cam) {
	glm::vec3 CameraDirection = cam->GetDirection();

	Renderer::m_ProceduralSkyShader->Bind();
	Renderer::m_ProceduralSkyShader->SetUniform1f("SurfaceRadius", SurfaceRadius);
	Renderer::m_ProceduralSkyShader->SetUniform1f("AtmosphereRadius", AtmosphereRadius);
	Renderer::m_ProceduralSkyShader->SetUniform1f("SunIntensity", SunIntensity);

	Renderer::m_ProceduralSkyShader->SetUniform3f("RayleighScattering", 
		RayleighScattering.r,
		RayleighScattering.g, 
		RayleighScattering.b);

	Renderer::m_ProceduralSkyShader->SetUniform3f("MieScattering",
		MieScattering.r,
		MieScattering.g,
		MieScattering.b);

	Renderer::m_ProceduralSkyShader->SetUniform3f("CenterPoint",
		CenterPoint.x,
		CenterPoint.y,
		CenterPoint.z);

	Renderer::m_ProceduralSkyShader->SetUniform3f("SunDirection",
		SunDirection.x,
		SunDirection.y,
		SunDirection.z);

	Renderer::m_ProceduralSkyShader->SetUniform1f("u_Exposure", cam->Exposure);

	//Renderer::m_ProceduralSkyShader->SetUniform3f("CamDirection",
	//	CameraDirection.x,
	//	CameraDirection.y,
	//	CameraDirection.z);
	

	Renderer::m_ProceduralSkyShader->SetUniformMat4f("InvProjection", cam->GetPerspective());
	Renderer::m_ProceduralSkyShader->SetUniformMat4f("InvView", cam->GetTransformRotation());
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
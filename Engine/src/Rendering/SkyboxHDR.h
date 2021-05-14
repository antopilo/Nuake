#pragma once
#include <glm\ext\vector_float3.hpp>
#include <glm\mat4x4.hpp>
#include "Textures/HDR.h"
#include <string>
class SkyboxHDR {

    static glm::vec3 vertices[]; 

	unsigned int VBO;
	unsigned int VAO;

public:
	HDRTexture* m_HDRTexture;
	unsigned int m_HDRtexture;
	unsigned int m_Cubemap;
	unsigned int m_ConvulatedCubemap;
	unsigned int m_SpecularCubemap;
	unsigned int m_brdLut;
	SkyboxHDR();
	SkyboxHDR(const std::string path);
	void Draw(glm::mat4 projection, glm::mat4 view);

	void Push();

	void CreateHDRCubemap();
	void CreateConvulatedCubemap();
	void CreateSpecularCubemaps();

	void CreateBRDLUT();

};
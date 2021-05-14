#include "HDR.h"
#include <GL\glew.h>
#include <iostream>
#include "../Renderer.h"

HDRTexture::HDRTexture(const std::string& path) {
	m_RendererId = 0;
	m_FilePath = path;
	m_LocalBuffer = nullptr;
	m_Width = 0;
	m_Height = 0;
	m_BPP = 0;

	stbi_set_flip_vertically_on_load(1);
	float* data = stbi_loadf(path.c_str(), &m_Width, &m_Height, &m_BPP, 0);

	if (data)
	{
		glGenTextures(1, &m_RendererId);
		glBindTexture(GL_TEXTURE_2D, m_RendererId);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, m_Width, m_Height, 0, GL_RGB, GL_FLOAT, data);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		stbi_image_free(data);
	}
	else
	{
		std::cout << "Failed to load HDR image." << std::endl;
	}
}

HDRTexture::~HDRTexture() {
	glDeleteTextures(1, &m_RendererId);
}

void HDRTexture::Bind(unsigned int slot) const {
	glActiveTexture(GL_TEXTURE0 + slot);
	glBindTexture(GL_TEXTURE_2D, m_RendererId);
}
void HDRTexture::BindCubemap(unsigned int slot) const {
	glActiveTexture(GL_TEXTURE0 + slot);
	glBindTexture(GL_TEXTURE_CUBE_MAP, m_CubemapId);
}

void HDRTexture::Unbind() const {
	glBindTexture(GL_TEXTURE_2D, 0);
}

void HDRTexture::GenerateCubeMap() {
	
}

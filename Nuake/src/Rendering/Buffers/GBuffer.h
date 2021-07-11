#pragma once
#include "glm/vec2.hpp"


class GBuffer
{
public:
	unsigned int m_FramebufferID;
	unsigned int m_RenderBuffer;

	unsigned int gDepth;
	unsigned int gAlbedo;
	unsigned int gNormal;
	unsigned int gMaterial;
	glm::vec2 m_Size;

	GBuffer(glm::vec2 size);

	void Bind();

	void Unbind();

	void Resize(glm::vec2 size);
	glm::vec2 GetSize();
};
#pragma once
#include "stb_image/stb_image.h"
#include <string>
#include "glm/vec2.hpp"
#include "msdfgen/core/BitmapRef.hpp"


typedef unsigned int GLenum;
class MultiSampledTexture
{
private:
	unsigned int m_RendererId;
	std::string m_FilePath;
	unsigned char* m_LocalBuffer;
	GLenum m_Format;

	int m_Width;
	int m_Height;
	int m_BPP; // byte per pixel.

public:
	MultiSampledTexture(const std::string& path);
	MultiSampledTexture(glm::vec2 size, msdfgen::BitmapConstRef<unsigned char, 4>& bitmap, bool t);
	MultiSampledTexture(glm::vec2 size, GLenum format);

	~MultiSampledTexture();

	void Resize(glm::vec2 size);
	void AttachToFramebuffer(GLenum attachment);

	void Bind(unsigned int slot = 0) const;
	void Unbind() const;

	unsigned int GetID() const { return m_RendererId; }

	inline int GetWidth() const { return m_Width; }
	inline int GetHeight() const { return m_Height; }
};




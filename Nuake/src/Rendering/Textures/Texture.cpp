#include "Texture.h"
#include "src/Core/Logger.h"
#include <GL\glew.h>
#include <iostream>
#include <src/Core/FileSystem.h>

namespace Nuake
{
	Texture::Texture(const std::string& path) 
	{
		m_RendererId = 0;
		m_FilePath = path;
		m_LocalBuffer = nullptr;
		m_Width = 0;
		m_Height = 0;
		m_BPP = 0;

		stbi_set_flip_vertically_on_load(1);
		m_LocalBuffer = stbi_load((path).c_str(), &m_Width, &m_Height, &m_BPP, 4);

		glGenTextures(1, &m_RendererId);
		glBindTexture(GL_TEXTURE_2D, m_RendererId);
		glGenerateMipmap(GL_TEXTURE_2D);
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		//glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_LOD_BIAS, -1.0f);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, m_Width, m_Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, m_LocalBuffer);
		glGenerateMipmap(GL_TEXTURE_2D);
		if (m_LocalBuffer)
		{
			stbi_image_free(m_LocalBuffer);
		}
		else
		{
			const std::string& msg = "Failed to load texture: " + path;
			Logger::Log(msg, "texture", WARNING);
		}
	}

	Texture::Texture(glm::vec2 size, GLenum format, GLenum format2, GLenum format3, void* data)
	{
		m_RendererId = 0;
		m_Format = format;

		m_Format2 = format;
		if (format2 != 0)
			m_Format2 = format2;

		m_Format3 = GL_UNSIGNED_BYTE;
		if (format3 != 0)
			m_Format3 = format3;

		m_Width = size.x;
		m_Height = size.y;

		glGenTextures(1, &m_RendererId);
		glBindTexture(GL_TEXTURE_2D, m_RendererId);
		glTexImage2D(GL_TEXTURE_2D, 0, format2, size.x, size.y, 0, format, format3, data);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
		//glGenerateMipmap(GL_TEXTURE_2D);
	}

	Texture::Texture(Vector2 size, unsigned char* data, int len)
	{
		m_RendererId = 0;
		m_Width = size.x;
		m_Height = size.y;
		int channels = 0;
		m_LocalBuffer = stbi_load_from_memory(data, len, &m_Width, &m_Height, &channels, 0);
		glGenTextures(1, &m_RendererId);
		glBindTexture(GL_TEXTURE_2D, m_RendererId);
		glGenerateMipmap(GL_TEXTURE_2D);
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_LOD_BIAS, -1.0f);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, m_Width, m_Height, 0, GL_RGB, GL_UNSIGNED_BYTE, m_LocalBuffer);
		glGenerateMipmap(GL_TEXTURE_2D);

		if (m_LocalBuffer)
		{
			stbi_image_free(m_LocalBuffer);
		}
		else
		{
			const std::string msg = "failed to load texture buffer";
			Logger::Log(msg, "texture", WARNING);
		}
	}

	Texture::Texture(glm::vec2 size, msdfgen::BitmapConstRef<unsigned char, 4>& bitmap, bool t)
	{
		m_RendererId = 0;
		m_Width = size.x;
		m_Height = size.y;

		glGenTextures(1, &m_RendererId);
		glBindTexture(GL_TEXTURE_2D, m_RendererId);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);


		auto pixel = bitmap(0, bitmap.height - 0 - 1);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, m_Width, m_Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, (unsigned char*)bitmap.pixels);
	}

	void Texture::Resize(glm::vec2 size)
	{

		glDeleteTextures(1, &m_RendererId);
		m_Width = size.x;
		m_Height = size.y;
		glGenTextures(1, &m_RendererId);
		glBindTexture(GL_TEXTURE_2D, m_RendererId);
		glTexImage2D(GL_TEXTURE_2D, 0, m_Format2, size.x, size.y, 0, m_Format, m_Format3, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}

	Texture::~Texture() 
	{
		glDeleteTextures(1, &m_RendererId);
	}

	void Texture::AttachToFramebuffer(GLenum attachment)
	{
		glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, attachment, GL_TEXTURE_2D, m_RendererId, 0);
	}

	void Texture::Bind(unsigned int slot) const 
	{
		glActiveTexture(GL_TEXTURE0 + slot);
		glBindTexture(GL_TEXTURE_2D, m_RendererId);
	}

	void Texture::Unbind() const 
	{
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	void Texture::SetParameter(const GLenum& param, const GLenum& value)
	{
		Bind();
		glTextureParameteri(GL_TEXTURE_2D, param, value);
		Unbind();
	}
}

#include "Texture.h"
#include "Nuake/Core/Logger.h"
#include "Nuake/FileSystem/FileSystem.h"
#include "Nuake/Core/String.h"

#include "dds/dds.h"

#include <glad/glad.h>

#include <fstream>
#include <iostream>


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

		if (String::EndsWith(path, ".dds"))
		{
			std::ifstream file(path, std::ios::binary | std::ios::ate);
			if (!file)
			{
				const std::string& msg = "Failed to load texture: " + path;
				Logger::Log(msg, "texture", WARNING);
				return;
			}

			size_t fileSize = file.tellg();
			std::vector<unsigned char> fileData(fileSize);
			file.seekg(0);
			file.read(reinterpret_cast<char*>(fileData.data()), fileSize);
			file.close();

			// Parse the DDS header
			dds::Header header = dds::read_header(fileData.data(), fileSize);
			if (!header.is_valid())
			{
				const std::string& msg = "Invalid DDS texture: " + path;
				Logger::Log(msg, "texture", WARNING);
				return;
			}

			// Set texture properties
			m_Width = header.width();
			m_Height = header.height();

		}
		else
		{
			stbi_set_flip_vertically_on_load(1);
			m_LocalBuffer = stbi_load((path).c_str(), &m_Width, &m_Height, &m_BPP, 4);
		}
		

		//glGenTextures(1, &m_RendererId);
		//glBindTexture(GL_TEXTURE_2D, m_RendererId);
		//glGenerateMipmap(GL_TEXTURE_2D);
		////glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		////glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		////glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_LOD_BIAS, -1.0f);
		//glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, m_Width, m_Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, m_LocalBuffer);
		//glGenerateMipmap(GL_TEXTURE_2D);
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

	Texture::Texture(const TextureFlags& flags, Vector2 size, void* data)
	{
		m_Width = size.x;
		m_Height = size.y;

		void* tempData = data;
		if (flags.flipVertical)
		{
			int row;
			size_t bytes_per_row = (size_t)m_Width * 4;
			stbi_uc temp[2048];
			stbi_uc* bytes = (stbi_uc*)tempData;

			for (row = 0; row < ((int)m_Width >> 1); row++) {
				stbi_uc* row0 = bytes + row * bytes_per_row;
				stbi_uc* row1 = bytes + ((int)m_Width - row - 1) * bytes_per_row;
				// swap row0 with row1
				size_t bytes_left = bytes_per_row;
				while (bytes_left) {
					size_t bytes_copy = (bytes_left < sizeof(temp)) ? bytes_left : sizeof(temp);
					memcpy(temp, row0, bytes_copy);
					memcpy(row0, row1, bytes_copy);
					memcpy(row1, temp, bytes_copy);
					row0 += bytes_copy;
					row1 += bytes_copy;
					bytes_left -= bytes_copy;
				}
			}
		}

		//glGenTextures(1, &m_RendererId);
		//glBindTexture(GL_TEXTURE_2D, m_RendererId);
		//glTexImage2D(GL_TEXTURE_2D, 0, (GLenum)flags.pixelFormat, size.x, size.y, 0, (GLenum)flags.pixelFormat, (GLenum)flags.pixelDataType, tempData);
		
		SetMagnificationFilter(flags.magFilter);
		SetMinificationFilter(flags.minFilter);
		SetWrapping(flags.wrapping);
	}

	Texture::Texture(Vector2 size, GLenum format, GLenum format2, GLenum format3, void* data)
	{
		m_RendererId = 0;
		m_Format = format;

		m_Format2 = format;
		if (format2 != 0)
			m_Format2 = format2;

		m_Filtering = GL_LINEAR;

		m_Format3 = GL_UNSIGNED_BYTE;
		if (format3 != 0)
			m_Format3 = format3;

		m_Width = size.x;
		m_Height = size.y;

		//glGenTextures(1, &m_RendererId);
		//glBindTexture(GL_TEXTURE_2D, m_RendererId);
		//glTexImage2D(GL_TEXTURE_2D, 0, format2, size.x, size.y, 0, format, format3, data);
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
		//glGenerateMipmap(GL_TEXTURE_2D);
	}

	Texture::Texture(unsigned char* data, int len)
	{
		m_RendererId = 0;
		int channels = 0;
		stbi_set_flip_vertically_on_load(1);
		m_LocalBuffer = stbi_load_from_memory(data, len, &m_Width, &m_Height, &channels, 4);
		//glGenTextures(1, &m_RendererId);
		//glBindTexture(GL_TEXTURE_2D, m_RendererId);
		//glGenerateMipmap(GL_TEXTURE_2D);
		//glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_Width, m_Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, m_LocalBuffer);
		//glGenerateMipmap(GL_TEXTURE_2D);

		if (m_LocalBuffer)
		{
			stbi_image_free(m_LocalBuffer);
		}
		else
		{
			const std::string msg = "Failed to load texture from buffer";
			Logger::Log(msg, "texture", WARNING);
		}
	}

	void Texture::Resize(glm::vec2 size)
	{

		//glDeleteTextures(1, &m_RendererId);
		m_Width = size.x;
		m_Height = size.y;
		//glGenTextures(1, &m_RendererId);
		//glBindTexture(GL_TEXTURE_2D, m_RendererId);
		//glTexImage2D(GL_TEXTURE_2D, 0, m_Format2, size.x, size.y, 0, m_Format, m_Format3, NULL);
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, m_Filtering);
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, m_Filtering);
	}

	Texture::~Texture() 
	{
		//glDeleteTextures(1, &m_RendererId);
	}

	void Texture::AttachToFramebuffer(GLenum attachment)
	{
		//glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, attachment, GL_TEXTURE_2D, m_RendererId, 0);
	}

	void Texture::Bind(unsigned int slot) const 
	{
		//glActiveTexture(GL_TEXTURE0 + slot);
		//glBindTexture(GL_TEXTURE_2D, m_RendererId);
	}

	void Texture::Unbind() const 
	{
		//glBindTexture(GL_TEXTURE_2D, 0);
	}

	void Texture::SetParameter(const GLenum& param, const GLenum& value)
	{
		if (param == GL_TEXTURE_MIN_FILTER)
		{
			m_Filtering = value;
		}

		Bind();
		//glTexParameteri(GL_TEXTURE_2D, param, value);
		Unbind();
	}

	void Texture::SetMagnificationFilter(const SamplerFilter& filter)
	{
		//glBindTexture(GL_TEXTURE_2D, m_RendererId);
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, (GLint)filter);
	}

	void Texture::SetMinificationFilter(const SamplerFilter& filter)
	{
		//glBindTexture(GL_TEXTURE_2D, m_RendererId);
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, (GLint)filter);
	}

	void Texture::SetWrapping(const SamplerWrapping& wrapping)
	{
		//glBindTexture(GL_TEXTURE_2D, m_RendererId);
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, (GLint)wrapping);
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, (GLint)wrapping);
	}

	json Texture::Serialize()
	{
		BEGIN_SERIALIZE();
		const std::string& relativePath = FileSystem::AbsoluteToRelative(m_FilePath);
		j["Path"] = relativePath;
		END_SERIALIZE();
	}

	bool Texture::Deserialize(const json& j)
	{
		if (j.contains("Path"))
			return false;

		return true;
	}

	void Texture::FlipOnLoad()
	{

	}
}

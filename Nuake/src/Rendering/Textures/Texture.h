#pragma once

#include "src/Core/Maths.h"
#include "src/Resource/Serializable.h"

#include "stb_image/stb_image.h"
#include "msdfgen/core/BitmapRef.hpp"

#include <string>

namespace Nuake
{
	typedef unsigned int GLenum;

	class Texture : ISerializable
	{
	private:
		unsigned int m_RendererId;
		std::string m_FilePath;
		unsigned char* m_LocalBuffer;
		GLenum m_Format;
		GLenum m_Format2;
		GLenum m_Format3;

		int m_Width;
		int m_Height;
		int m_BPP; // byte per pixel.

	public:
		Texture(const std::string& path); // Load texture from file
		Texture(unsigned char* data, int len); // Used to load texture from a memory buffer

		Texture(Vector2 size, GLenum format, GLenum format2 = 0, GLenum format3 = 0, void* data = 0); // Used to load texture from memeory with known size
		Texture(Vector2 size, msdfgen::BitmapConstRef<unsigned char, 4>& bitmap, bool t); // Used internally for MSDF fonts
		~Texture();

		void Resize(glm::vec2 size);
		void AttachToFramebuffer(GLenum attachment);

		void Bind(unsigned int slot = 0) const;
		void Unbind() const;

		void SetParameter(const GLenum& param, const GLenum& value);

		unsigned int GetID() const { return m_RendererId; }
		inline std::string GetPath() { return m_FilePath; }
		inline int GetWidth() const { return m_Width; }
		inline int GetHeight() const { return m_Height; }
		inline Vector2 GetSize() const { return Vector2(m_Width, m_Height); }

		json Serialize() override;
		bool Deserialize(const json& j) override;
	};
}

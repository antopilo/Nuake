#pragma once

#include "Nuake/Core/Maths.h"
#include "Nuake/Resource/Serializable.h"

#include "stb_image/stb_image.h"

#include <string>

namespace Nuake
{
	typedef unsigned int GLenum;

	enum class PixelFormat
	{	// TODO: ADD MORE FORMATS
		RGB8 = 0x1907,
		RGBA8 = 0x1908
	};

	enum class PixelDataType
	{	// TODO: ADD MORE FORMATS
		UBYTE = 0x1401,
		FLOAT = 0x1406
	};

	enum class SamplerFilter
	{	// TODO: ADD MORE FILTERS
		LINEAR = 0x2601,
		NEAREST = 0x2600
	};

	enum class SamplerWrapping
	{	// TODO: ADD MORE WRAPPING
		CLAMP_TO_EDGE = 0x2900,
		REPEAT = 0x2901
	};

	struct TextureFlags
	{
		PixelFormat pixelFormat = PixelFormat::RGBA8;
		PixelDataType pixelDataType = PixelDataType::UBYTE;

		SamplerFilter minFilter = SamplerFilter::LINEAR;
		SamplerFilter magFilter = SamplerFilter::LINEAR;
		SamplerWrapping wrapping = SamplerWrapping::REPEAT;
		bool flipVertical = false;
	};

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

		GLenum m_Filtering;

	public:
		Texture(const std::string& path); // Load texture from file
		Texture(unsigned char* data, int len); // Used to load texture from a memory buffer
		Texture(const TextureFlags& flags, Vector2 size, void* data);
		Texture(Vector2 size, GLenum format, GLenum format2 = 0, GLenum format3 = 0, void* data = 0); // Used to load texture from memeory with known size
		~Texture();

		void Resize(glm::vec2 size);
		void AttachToFramebuffer(GLenum attachment);

		void Bind(unsigned int slot = 0) const;
		void Unbind() const;

		void SetParameter(const GLenum& param, const GLenum& value);

		void SetMagnificationFilter(const SamplerFilter& filter);
		void SetMinificationFilter(const SamplerFilter& filter);
		void SetWrapping(const SamplerWrapping& wrapping);

		unsigned int GetID() const { return m_RendererId; }
		inline std::string GetPath() { return m_FilePath; }
		inline int GetWidth() const { return m_Width; }
		inline int GetHeight() const { return m_Height; }
		inline Vector2 GetSize() const { return Vector2(m_Width, m_Height); }

		json Serialize() override;
		bool Deserialize(const json& j) override;

	private:
		void FlipOnLoad();
	};
}

#pragma once
#include "stb_image/stb_image.h"
#include <string>


class HDRTexture
{
private:
	unsigned int m_RendererId;
	std::string m_FilePath;
	unsigned char* m_LocalBuffer;
	int m_Width;
	int m_Height;
	int m_BPP; // byte per pixel.
	unsigned int m_CubemapId;

public:
	HDRTexture(const std::string& path);
	~HDRTexture();

	void Bind(unsigned int slot = 0) const;
	void BindCubemap(unsigned int slot = 0) const;
	void Unbind() const;

	inline int GetWidth() const { return m_Width; }
	inline int GetHeight() const { return m_Height; }

	void GenerateCubeMap();

	void SetCubemapId(unsigned int id) { m_CubemapId = id; }
	unsigned int GetCubemapId() { return m_CubemapId; }
};




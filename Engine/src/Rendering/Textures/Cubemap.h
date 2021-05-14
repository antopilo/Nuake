#pragma once
#include "stb_image/stb_image.h"
#include <string>


class CubemapTexture
{
private:
	unsigned int m_RendererId;
	std::string m_FilePath;
	unsigned char* m_LocalBuffer;
	int m_Width;
	int m_Height;
	int m_BPP; // byte per pixel.

public:
	CubemapTexture(const std::string& path);
	void CreateFromHDR();
	~CubemapTexture();

	void Bind(unsigned int slot = 0) const;
	void Unbind() const;

	inline int GetWidth() const { return m_Width; }
	inline int GetHeight() const { return m_Height; }
};




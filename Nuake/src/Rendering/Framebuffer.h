#pragma once
#include "../Core/Maths.h"
#include "Textures/Texture.h"
#include <vector>
#include "../Core/Core.h"
#include <map>
class FrameBuffer
{
private:
	unsigned int m_FramebufferID;
	unsigned int m_RenderBuffer;

	Vector2 m_Size;

	std::map<int, Ref<Texture>> m_Textures;
	Ref<Texture> m_Texture;

public:
	FrameBuffer(bool hasRenderBuffer, Vector2 size);
	~FrameBuffer() { }

	// 0x8CE0 = color attachment 0.
	// TODO: Remove blackbox crap
	Ref<Texture> GetTexture(GLenum attachment = 0x8CE0) { return m_Textures[(int)attachment]; }
	void SetTexture(Ref<Texture> texture, GLenum attachment = 0x8CE0);


	void Bind();
	void Unbind();
	Vector2 GetSize() const { return m_Size; }
	void UpdateSize(Vector2 size);

	void SetDrawBuffer(GLenum draw);
	void SetReadBuffer(GLenum read);
};
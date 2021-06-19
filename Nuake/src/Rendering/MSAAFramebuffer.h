#pragma once
#include "../Core/Maths.h"
#include <vector>
#include "../Core/Core.h"

#include <src/Rendering/Textures/MultiSampledTexture.h>
#include <map>
class MSAAFrameBuffer
{
private:
	unsigned int m_FramebufferID;
	unsigned int m_RenderBuffer;

	Vector2 m_Size;
	bool ResizeQueued = false;

	std::map<int, Ref<MultiSampledTexture>> m_Textures;
	Ref<MultiSampledTexture> m_Texture;

public:
	MSAAFrameBuffer(bool hasRenderBuffer, Vector2 size);
	~MSAAFrameBuffer() { }

	// 0x8CE0 = color attachment 0.
	// TODO: Remove blackbox crap
	Ref<MultiSampledTexture> GetTexture(GLenum attachment = 0x8CE0) { return m_Textures[(int)attachment]; }
	void SetTexture(Ref<MultiSampledTexture> texture, GLenum attachment = 0x8CE0);


	void Bind();
	void Unbind();
	void QueueResize(Vector2 size);
	Vector2 GetSize() const { return m_Size; }
	void UpdateSize(Vector2 size);

	void SetDrawBuffer(GLenum draw);
	void SetReadBuffer(GLenum read);
};
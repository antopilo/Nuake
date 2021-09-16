#pragma once
#include "../Core/Maths.h"

#include <vector>
#include "../Core/Core.h"
#include <map>
#include <src/Rendering/Textures/Texture.h>

namespace Nuake
{
	class FrameBuffer
	{
	private:
		unsigned int m_FramebufferID;
		unsigned int m_RenderBuffer;

		Vector2 m_Size;
		bool ResizeQueued = false;

		std::map<int, Ref<Texture>> m_Textures;
		Ref<Texture> m_Texture;
		bool m_HasRenderBuffer = false;

	public:
		FrameBuffer(bool hasRenderBuffer, Vector2 size);
		~FrameBuffer();

		// 0x8CE0 = color attachment 0.
		// TODO: Remove blackbox crap
		Ref<Texture> GetTexture(GLenum attachment = 0x8CE0) { return m_Textures[(int)attachment]; }
		void SetTexture(Ref<Texture> texture, GLenum attachment = 0x8CE0);

		void Clear();
		void Bind();
		void Unbind();
		void QueueResize(Vector2 size);
		Vector2 GetSize() const { return m_Size; }
		void SetSize(const Vector2& size) { m_Size = size; }
		void UpdateSize(Vector2 size);

		void SetDrawBuffer(GLenum draw);
		void SetReadBuffer(GLenum read);
	};
}
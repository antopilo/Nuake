#include "Framebuffer.h"
#include <GL\glew.h>

namespace Nuake
{
	FrameBuffer::FrameBuffer(bool hasRenderBuffer, Vector2 size)
	{
		m_Textures = std::map<int, Ref<Texture>>();
		m_Size = size;

		glGenFramebuffers(1, &m_FramebufferID);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_FramebufferID);

		// Create render buffer and attach to frame buffer.
		if (hasRenderBuffer)
		{
			glGenRenderbuffers(1, &m_RenderBuffer);
			glBindRenderbuffer(GL_RENDERBUFFER, m_RenderBuffer);
			glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, m_Size.x, m_Size.y);
			glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_RenderBuffer);
		}
		else
		{
			m_RenderBuffer = -1;
		}

		// Unbind
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	}

	FrameBuffer::~FrameBuffer()
	{
		glDeleteFramebuffers(1, &m_FramebufferID);
		glDeleteRenderbuffers(1, &m_RenderBuffer);
	}

	void FrameBuffer::SetTexture(Ref<Texture> texture, GLenum attachment)
	{
		m_Textures[attachment] = texture;
		// Attach texture to the framebuffer.
		Bind();
		texture->AttachToFramebuffer(attachment);

		// Set draw buffer with dynamic amount of render target.
		// Surely, this can be optimized.
		int size = 0;
		std::vector<unsigned int> keys = std::vector<unsigned int>();
		for (auto& s : m_Textures)
		{
			if (s.first == GL_DEPTH_ATTACHMENT)
				continue;

			keys.push_back(s.first);
			size += 1;
		}

		if (size > 0)
			glDrawBuffers(size, &keys[0]);

		Unbind();
	}


	void FrameBuffer::Clear()
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	void FrameBuffer::Bind()
	{
		if (ResizeQueued)
			UpdateSize(m_Size);

		glBindFramebuffer(GL_FRAMEBUFFER, m_FramebufferID);

		glViewport(0, 0, m_Size.x, m_Size.y);
	}

	void FrameBuffer::Unbind()
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void FrameBuffer::QueueResize(Vector2 size)
	{
		ResizeQueued = true;
		m_Size = size;
	}

	void FrameBuffer::UpdateSize(Vector2 size)
	{
		m_Size = size;
		ResizeQueued = false;

		// Delete frame buffer and render buffer.
		glDeleteFramebuffers(1, &m_FramebufferID);
		glDeleteRenderbuffers(1, &m_RenderBuffer);

		// Recreate resized texture.
		for (auto t : m_Textures)
			t.second->Resize(size);

		// New FBO and RBO.
		glGenFramebuffers(1, &m_FramebufferID);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_FramebufferID);

		// Recreate resized texture.
		for (auto t : m_Textures)
			t.second->AttachToFramebuffer(t.first);

		// Recreate render buffer
		glGenRenderbuffers(1, &m_RenderBuffer);
		glBindRenderbuffer(GL_RENDERBUFFER, m_RenderBuffer);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, m_Size.x, m_Size.y);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_RenderBuffer);

		// Unbind.
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	}

	void FrameBuffer::SetDrawBuffer(GLenum draw)
	{
		Bind();
		//glReadBuffer(draw);
		Unbind();
	}

	void FrameBuffer::SetReadBuffer(GLenum read)
	{
		Bind();
		//glReadBuffer(read);
		Unbind();
	}
}

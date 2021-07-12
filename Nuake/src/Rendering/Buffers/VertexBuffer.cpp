#include "VertexBuffer.h"

namespace Nuake {
	VertexBuffer::VertexBuffer(const void* data, unsigned int size, RendererEnum bufferType)
	{
		m_BufferType = bufferType;
		RenderCommand::GenBuffer(m_RendererID);
		Bind();
		RenderCommand::SetBufferData(m_BufferType, data, size);
	}

	VertexBuffer::~VertexBuffer()
	{
		RenderCommand::DeleteBuffer(m_RendererID);
	}

	void VertexBuffer::Bind() const
	{
		RenderCommand::BindBuffer(m_BufferType, m_RendererID);
	}

	void VertexBuffer::Unbind() const
	{
		RenderCommand::BindBuffer(m_BufferType, 0);
	}
}
#include "VertexBuffer.h"

namespace Nuake {
	VertexBuffer::VertexBuffer(const void* data, unsigned int size, RendererEnum bufferType, RendererEnum dataHint)
	{
		m_BufferType = bufferType;
		RenderCommand::GenBuffer(m_RendererID);
		Bind();
		RenderCommand::SetBufferData(m_BufferType, data, size, dataHint);
	}

	VertexBuffer::~VertexBuffer()
	{
		RenderCommand::DeleteBuffer(m_RendererID);
	}

	void VertexBuffer::SetSubData(const void* data, unsigned int size, unsigned int offset)
	{
		RenderCommand::SetBufferSubData(m_BufferType, data, size, offset);
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
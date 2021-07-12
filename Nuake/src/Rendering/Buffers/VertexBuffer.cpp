#include "VertexBuffer.h"
#include "src/Rendering/RenderCommand.h"

namespace Nuake {
	VertexBuffer::VertexBuffer(const void* data, unsigned int size)
	{
		RenderCommand::GenBuffer(m_RendererID);
		Bind();
		RenderCommand::SetBufferData(RendererEnum::ARRAY_BUFFER, data, size);
	}

	VertexBuffer::~VertexBuffer()
	{
		RenderCommand::DeleteBuffer(m_RendererID);
	}

	void VertexBuffer::Bind() const
	{
		RenderCommand::BindBuffer(RendererEnum::ARRAY_BUFFER, m_RendererID);
	}

	void VertexBuffer::Unbind() const
	{
		RenderCommand::BindBuffer(RendererEnum::ARRAY_BUFFER, 0);
	}
}
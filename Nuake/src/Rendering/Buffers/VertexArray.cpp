#include "VertexArray.h"
#include "src/Rendering/RenderCommand.h"

namespace Nuake {
	VertexArray::VertexArray()
	{
		RenderCommand::GenVertexArray(m_RendererID);
	}

	VertexArray::~VertexArray()
	{
		RenderCommand::DeleteVertexArray(m_RendererID);
	}

	void VertexArray::AddBuffer(const VertexBuffer& vb, const VertexBufferLayout& layout)
	{
		Bind();
		vb.Bind();

		const auto& elements = layout.GetElements();
		unsigned int offset = 0;
		for (unsigned int i = 0; i < elements.size(); i++)
		{
			const auto& element = elements[i];
			RenderCommand::EnableVertexAttribArray(i);
			RenderCommand::VertexAttribPointer(i, element.count, element.type, element.normalized, layout.GetStride(), (const void*)(uintptr_t)offset);
			offset += element.count * VertexBufferElement::GetSizeOfType(element.type);
		}
	}

	void VertexArray::Bind() const
	{
		RenderCommand::BindVertexArray(m_RendererID);
	}

	void VertexArray::Unbind() const
	{
		RenderCommand::BindVertexArray(0);
	}
}
#pragma once
#include "Nuake/Rendering/RenderCommand.h"

namespace Nuake {
	class VertexBuffer
	{
	public:
		VertexBuffer() {}
		VertexBuffer(const void* data, unsigned int size, RendererEnum bufferType = RendererEnum::ARRAY_BUFFER, RendererEnum dataHint = RendererEnum::STATIC_DRAW);
		~VertexBuffer();

		void SetSubData(const void* data, unsigned int size, unsigned int offset = 0);

		void Bind() const;
		void Unbind() const;
	private:
		unsigned int m_RendererID = -1;
		RendererEnum m_BufferType;
	};
}
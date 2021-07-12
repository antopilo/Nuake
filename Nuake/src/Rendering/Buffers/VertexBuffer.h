#pragma once
#include "src/Rendering/RenderCommand.h"

namespace Nuake {
	class VertexBuffer
	{
	public:
		VertexBuffer() {}
		VertexBuffer(const void* data, unsigned int size, RendererEnum bufferType = RendererEnum::ARRAY_BUFFER);
		~VertexBuffer();

		void Bind() const;
		void Unbind() const;
	private:
		unsigned int m_RendererID = -1;
		RendererEnum m_BufferType;
	};
}
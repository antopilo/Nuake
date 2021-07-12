#pragma once

namespace Nuake {
	class VertexBuffer
	{
	public:
		VertexBuffer(const void* data, unsigned int size);
		~VertexBuffer();

		void Bind() const;
		void Unbind() const;
	private:
		unsigned int m_RendererID = -1;
	};
}
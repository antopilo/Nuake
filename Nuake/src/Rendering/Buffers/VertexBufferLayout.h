#pragma once
#include <vector>
#include "src/Rendering/Platforms/RendererAPI.h"

namespace Nuake {
	struct VertexBufferElement
	{
		RendererEnum type;
		unsigned int count;
		bool normalized;

		static unsigned int GetSizeOfType(RendererEnum type)
		{
			switch (type)
			{
				case RendererEnum::FLOAT:	return 4;
				case RendererEnum::INT:		return 4;
				case RendererEnum::UINT:	return 4;
				case RendererEnum::UBYTE:	return 1;
			}
			return 0;
		}
	};

	class VertexBufferLayout
	{
	private:
		std::vector<VertexBufferElement> m_Elements;
		unsigned int m_Stride;

	public:
		VertexBufferLayout() : m_Stride(0) {};

		template<typename T>
		void Push(unsigned int count)
		{
			static_assert(false);
		}

		template<>
		void Push<float>(unsigned int count)
		{
			m_Elements.push_back({ RendererEnum::FLOAT, count, false });
			m_Stride += VertexBufferElement::GetSizeOfType(RendererEnum::FLOAT) * count;
		}

		template<>
		void Push<unsigned int>(unsigned int count)
		{
			m_Elements.push_back({ RendererEnum::UINT, count, false });
			m_Stride += VertexBufferElement::GetSizeOfType(RendererEnum::UINT) * count;
		}

		template<>
		void Push<int>(unsigned int count)
		{
			m_Elements.push_back({ RendererEnum::INT, count, false });
			m_Stride += VertexBufferElement::GetSizeOfType(RendererEnum::INT) * count;
		}

		template<>
		void Push<unsigned char>(unsigned int count)
		{
			m_Elements.push_back({ RendererEnum::UBYTE, count, true });
			m_Stride += VertexBufferElement::GetSizeOfType(RendererEnum::UBYTE) * count;
		}

		inline const std::vector<VertexBufferElement>& GetElements() const { return m_Elements; }

		inline unsigned int GetStride() const { return m_Stride; }
	};
}
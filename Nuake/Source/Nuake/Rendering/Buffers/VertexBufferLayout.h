#pragma once
#include <vector>
#include "Nuake/Rendering/Platforms/RendererAPI.h"

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
		VertexBufferLayout() : m_Stride(0) {}

#ifdef NK_WIN
		template<typename T>
		void Push(unsigned int count)
		{
			if constexpr (std::is_same_v<float, T>)
			{
				m_Elements.push_back({ RendererEnum::FLOAT, count, false });
				m_Stride += VertexBufferElement::GetSizeOfType(RendererEnum::FLOAT) * count;
			}
			else if constexpr (std::is_same_v<unsigned int, T>)
			{
				m_Elements.push_back({ RendererEnum::UINT, count, false });
				m_Stride += VertexBufferElement::GetSizeOfType(RendererEnum::UINT) * count;
			}
			else if constexpr (std::is_same_v<int, T>)
			{
				m_Elements.push_back({ RendererEnum::INT, count, false });
				m_Stride += VertexBufferElement::GetSizeOfType(RendererEnum::INT) * count;
			}
		}	
#endif
#ifdef NK_LINUX
		template<typename T>
		void Push(unsigned int count)
		{
			if constexpr (std::is_same_v<float, T>)
			{
				m_Elements.push_back({ RendererEnum::FLOAT, count, false });
				m_Stride += VertexBufferElement::GetSizeOfType(RendererEnum::FLOAT) * count;
			}
			else if constexpr (std::is_same_v<unsigned int, T>)
			{
				m_Elements.push_back({ RendererEnum::UINT, count, false });
				m_Stride += VertexBufferElement::GetSizeOfType(RendererEnum::UINT) * count;
			}
			else if constexpr (std::is_same_v<int, T>)
			{
				m_Elements.push_back({ RendererEnum::INT, count, false });
				m_Stride += VertexBufferElement::GetSizeOfType(RendererEnum::INT) * count;
			}
		}
#endif
		inline const std::vector<VertexBufferElement>& GetElements() const { return m_Elements; }

		inline unsigned int GetStride() const { return m_Stride; }
	};
}
#include "VKRendererAPI.h"
#include "glad/glad.h"
#include <volk/volk.h>

namespace Nuake {

	void VKRendererAPI::Enable(const RendererEnum enumType)
	{
	}

	void VKRendererAPI::Disable(const RendererEnum enumType)
	{
	}

	void VKRendererAPI::Clear()
	{
		
	}

	void VKRendererAPI::SetClearColor(const Color& color)
	{
	}

	void VKRendererAPI::GenBuffer(unsigned int& bufferID)
	{
	}

	void VKRendererAPI::BindBuffer(const RendererEnum bufferType, const unsigned int& bufferID)
	{
	}

	void VKRendererAPI::SetBufferData(const RendererEnum bufferType, const void* data, unsigned int size, const RendererEnum bufferDataHint)
	{
	}

	void VKRendererAPI::SetBufferSubData(const RendererEnum bufferType, const void* data, unsigned int size, unsigned int offset)
	{
	}

	void VKRendererAPI::DeleteBuffer(const unsigned int& bufferID)
	{
	}

	void VKRendererAPI::GenVertexArray(unsigned int& rendererID)
	{
	}

	void VKRendererAPI::DeleteVertexArray(unsigned int& rendererID)
	{
	}

	void VKRendererAPI::BindVertexArray(const unsigned int& rendererID)
	{
	}

	void VKRendererAPI::EnableVertexAttribArray(unsigned int& index)
	{
	}

	void VKRendererAPI::VertexAttribPointer(const unsigned int index, const int size, const RendererEnum type, bool normalized, int stride, const void* pointer)
	{

	}

	void VKRendererAPI::DrawMultiElements(const RendererEnum mode, const int count, const RendererEnum type, const void* const* indices, unsigned int drawCount)
	{
	}

	void VKRendererAPI::DrawElements(const RendererEnum mode, const uint32_t count, const RendererEnum type, const void* indices)
	{
	}

	void VKRendererAPI::DrawArrays(int from, int count)
	{
	}

	void VKRendererAPI::DrawLines(int from, int count)
	{
	}
}
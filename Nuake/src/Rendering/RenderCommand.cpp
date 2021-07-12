#include "RenderCommand.h"
#include "Platforms/OGLRendererAPI.h"

namespace Nuake {
	RendererAPI* RenderCommand::sRendererAPI;

	void RenderCommand::SetRendererAPI(RendererPlatforms platform)
	{
		sRendererAPI = new OGLRendererAPI();
	}

	void RenderCommand::Clear()
	{
		sRendererAPI->Clear();
	}

	void RenderCommand::SetClearColor(const Color& color)
	{
		sRendererAPI->SetClearColor(color);
	}

	void RenderCommand::GenBuffer(unsigned int& bufferID)
	{
		sRendererAPI->GenBuffer(bufferID);
	}

	void RenderCommand::BindBuffer(const RendererEnum bufferType, const unsigned int& bufferID)
	{
		sRendererAPI->BindBuffer(bufferType, bufferID);
	}

	void RenderCommand::SetBufferData(const RendererEnum bufferType, const void* data, unsigned int size)
	{
		sRendererAPI->SetBufferData(bufferType, data, size);
	}

	void RenderCommand::DeleteBuffer(unsigned int bufferID)
	{
		sRendererAPI->DeleteBuffer(bufferID);
	}

	void RenderCommand::GenVertexArray(unsigned int& rendererID)
	{
		sRendererAPI->GenVertexArray(rendererID);
	}

	void RenderCommand::DeleteVertexArray(unsigned int& rendererID)
	{
		sRendererAPI->DeleteVertexArray(rendererID);
	}

	void RenderCommand::EnableVertexAttribArray(unsigned int slot)
	{
		sRendererAPI->EnableVertexAttribArray(slot);
	}

	void RenderCommand::BindVertexArray(const unsigned int& rendererID)
	{
		sRendererAPI->BindVertexArray(rendererID);
	}

	void RenderCommand::VertexAttribPointer(const unsigned int index, const int size, const RendererEnum type, bool normalized, int stride, const void* pointer)
	{
		sRendererAPI->VertexAttribPointer(index, size, type, normalized, stride, pointer);
	}

	void RenderCommand::DrawArrays(int first, int count)
	{
		sRendererAPI->DrawArrays(first, count);
	}
}
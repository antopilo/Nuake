#include "RenderCommand.h"
#include "Platforms/OGLRendererAPI.h"
#include "Platforms/VKRendererAPI.h"


namespace Nuake {
	RendererAPI* RenderCommand::sRendererAPI;

	void RenderCommand::SetRendererAPI(RendererPlatforms platform)
	{
		if (platform == RendererPlatforms::OpenGL)
		{
			sRendererAPI = new OGLRendererAPI();
		}
		else if (platform == RendererPlatforms::Vulkan)
		{
			sRendererAPI = new VKRendererAPI();
		}
	}

	void RenderCommand::Clear()
	{
		sRendererAPI->Clear();
	}

	void RenderCommand::SetClearColor(const Color& color)
	{
		sRendererAPI->SetClearColor(color);
	}

	void RenderCommand::Enable(const RendererEnum enumType)
	{
		sRendererAPI->Enable(enumType);
	}

	void RenderCommand::Disable(const RendererEnum enumType)
	{
		sRendererAPI->Disable(enumType);
	}

	void RenderCommand::GenBuffer(unsigned int& bufferID)
	{
		sRendererAPI->GenBuffer(bufferID);
	}

	void RenderCommand::BindBuffer(const RendererEnum bufferType, const unsigned int& bufferID)
	{
		sRendererAPI->BindBuffer(bufferType, bufferID);
	}

	void RenderCommand::SetBufferData(const RendererEnum bufferType, const void* data, unsigned int size, const RendererEnum dataHint)
	{
		sRendererAPI->SetBufferData(bufferType, data, size, dataHint);
	}

	void RenderCommand::SetBufferSubData(const RendererEnum bufferType, const void* data, unsigned int size, unsigned int offset)
	{
		sRendererAPI->SetBufferSubData(bufferType, data, size, offset);
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

	void RenderCommand::DrawMultiElements(const RendererEnum mode, const int count, const RendererEnum type, const void* const* indices, unsigned int drawCount)
	{
		sRendererAPI->DrawMultiElements(mode, count, type, indices, drawCount);
	}

	void RenderCommand::DrawElements(const RendererEnum mode, const uint32_t count, const RendererEnum type, const void* indices)
	{
		sRendererAPI->DrawElements(mode, count, type, indices);
	}


	void RenderCommand::DrawArrays(int first, int count)
	{
		sRendererAPI->DrawArrays(first, count);
	}

	void RenderCommand::DrawLines(int first, int count)
	{
		sRendererAPI->DrawLines(first, count);
	}
}
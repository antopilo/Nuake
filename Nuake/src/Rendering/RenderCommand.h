#pragma once
#include "src/Rendering/Platforms/RendererAPI.h"

namespace Nuake {
	enum class RendererPlatforms
	{
		OpenGL
	};

	class RenderCommand 
	{
	public:
		static void SetRendererAPI(RendererPlatforms platform);

		static void Clear();
		static void SetClearColor(const Color& color);

		static void GenBuffer(unsigned int& bufferID);
		static void BindBuffer(const RendererEnum bufferType, const unsigned int& bufferID);
		static void SetBufferData(const RendererEnum bufferType, const void* data, unsigned int size);
		static void DeleteBuffer(unsigned int bufferID);
		
		static void GenVertexArray(unsigned int& rendererID);
		static void DeleteVertexArray(unsigned int& rendererID);
		static void EnableVertexAttribArray(unsigned int slot);

		static void BindVertexArray(const unsigned int& rendererID);
		static void VertexAttribPointer(const unsigned int index, const int size, const RendererEnum type, bool normalized, int stride, const void* pointer);
		static void DrawMultiElements(const RendererEnum mode, const int count, const RendererEnum type, const void* const* indices, unsigned int drawCount);
		static void DrawElements(const RendererEnum mode, const int count, const RendererEnum type, const void* indices);
		static void DrawArrays(int first, int count);
	private:
		static RendererAPI* sRendererAPI;
	};
}
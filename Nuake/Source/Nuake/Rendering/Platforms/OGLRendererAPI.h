#pragma once
#include "RendererAPI.h"

typedef unsigned int GLenum;

namespace Nuake {

	class OGLRendererAPI : public RendererAPI
	{
	public:
		OGLRendererAPI() {};

		void Enable(const RendererEnum enumType) override;
		void Disable(const RendererEnum enumType) override;

		void Clear() override;
		void SetClearColor(const Color& color) override;

		void GenBuffer(unsigned int& bufferID) override;
		void BindBuffer(const RendererEnum bufferType, const unsigned int& bufferID) override;
		void SetBufferData(const RendererEnum bufferType, const void* data, unsigned int size, const RendererEnum drawType) override;
		void SetBufferSubData(const RendererEnum bufferType, const void* data, unsigned int size, unsigned int offset = 0) override;
		void DeleteBuffer(const unsigned int& bufferID) override;

		void GenVertexArray(unsigned int& rendererID) override;
		void DeleteVertexArray(unsigned int& rendererID) override;
		void BindVertexArray(const unsigned int& rendererID) override;
		void EnableVertexAttribArray(unsigned int& index) override;
		void VertexAttribPointer(const unsigned int index, const int size, const RendererEnum type, bool normalized, int stride, const void* pointer) override;

		void DrawMultiElements(const RendererEnum mode, const int count, const RendererEnum type, const void* const* indices, unsigned int drawCount) override;
		void DrawElements(const RendererEnum mode, const uint32_t count, const RendererEnum type, const void* indices) override;
		void DrawArrays(int from, int count) override;

		void DrawLines(int from, int count) override;

	private:
		GLenum GetType(const RendererEnum& bufferType);
	};
}
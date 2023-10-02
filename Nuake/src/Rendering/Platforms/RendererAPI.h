#pragma once
#include "src/Core/Maths.h"

namespace Nuake {
	enum class RendererEnum {
		INT, UINT, BYTE, UBYTE,
		FLOAT, UFLOAT,
		ARRAY_BUFFER,
		ELEMENT_ARRAY_BUFFER,
		TRIANGLES, LINES,
		DEPTH_ATTACHMENT, COLOR_ATTACHMENT0, COLOR_ATTACHMENT1,
		DEPTH_TEST, FACE_CULL, BLENDING,
		STATIC_DRAW, DYNAMIC_DRAW, STREAM_DRAW
	};

	class RendererAPI
	{
	public:
		virtual void Clear() = 0;
		virtual void SetClearColor(const Color& color) = 0;

		virtual void Enable(const RendererEnum enumType) = 0;
		virtual void Disable(const RendererEnum enumType) = 0;

		virtual void GenBuffer(unsigned int& bufferID) = 0;
		virtual void BindBuffer(const RendererEnum bufferType, const unsigned int& bufferID) = 0;
		virtual void SetBufferData(const RendererEnum bufferType, const void* data, unsigned int size, const RendererEnum bufferDataHint = RendererEnum::STATIC_DRAW) = 0;
		virtual void SetBufferSubData(const RendererEnum bufferType, const void* data, unsigned int size, unsigned int offset = 0) = 0;
		virtual void DeleteBuffer(const unsigned int& bufferID) = 0;

		virtual void GenVertexArray(unsigned int& rendererID) = 0;
		virtual void DeleteVertexArray(unsigned int& rendererID) = 0;
		virtual void BindVertexArray(const unsigned int& rendererID) = 0;
		virtual void EnableVertexAttribArray(unsigned int& index) = 0;
		virtual void VertexAttribPointer(const unsigned int index, const int size, const RendererEnum type, bool normalized, int stride, const void* pointer) = 0;

		virtual void DrawMultiElements(const RendererEnum mode, const int count, const RendererEnum type, const void* const* indices, unsigned int drawCount) = 0;
		virtual void DrawElements(const RendererEnum mode, const uint32_t count, const RendererEnum type, const void* indices) = 0;
		virtual void DrawArrays(int from, int count) = 0;
		virtual void DrawLines(int from, int count) = 0;
	};
}
#include "OGLRendererAPI.h"
#include "GL/glew.h"

namespace Nuake {
	void OGLRendererAPI::Enable(const RendererEnum enumType)
	{
		glEnable(GetType(enumType));
	}

	void OGLRendererAPI::Disable(const RendererEnum enumType)
	{
		glDisable(GetType(enumType));
	}

	void OGLRendererAPI::Clear()
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	void OGLRendererAPI::SetClearColor(const Color& color)
	{
		glClearColor(color.r, color.g, color.g, color.a);
	}

	void OGLRendererAPI::GenBuffer(unsigned int& bufferID)
	{
		glGenBuffers(1, &bufferID);
	}

	void OGLRendererAPI::BindBuffer(const RendererEnum bufferType, const unsigned int& bufferID)
	{
		glBindBuffer(GetType(bufferType), bufferID);
	}

	void OGLRendererAPI::SetBufferData(const RendererEnum bufferType, const void* data, unsigned int size)
	{
		glBufferData(GetType(bufferType), size, data, GL_STATIC_DRAW);
	}

	void OGLRendererAPI::DeleteBuffer(const unsigned int& bufferID)
	{
		glDeleteBuffers(1, &bufferID);
	}

	void OGLRendererAPI::GenVertexArray(unsigned int& rendererID)
	{
		glGenVertexArrays(1, &rendererID);
	}

	void OGLRendererAPI::DeleteVertexArray(unsigned int& rendererID)
	{
		glDeleteVertexArrays(1, &rendererID);
	}

	void OGLRendererAPI::BindVertexArray(const unsigned int& rendererID)
	{
		glBindVertexArray(rendererID);
	}

	void OGLRendererAPI::EnableVertexAttribArray(unsigned int& index)
	{
		glEnableVertexAttribArray(index);
	}

	void OGLRendererAPI::VertexAttribPointer(const unsigned int index, const int size, const RendererEnum type, bool normalized, int stride, const void* pointer)
	{
		GLenum glType = GetType(type);
		glVertexAttribPointer(index, size, glType, normalized, stride, pointer);
	}

	void OGLRendererAPI::DrawMultiElements(const RendererEnum mode, const int count, const RendererEnum type, const void* const* indices, unsigned int drawCount)
	{
		glMultiDrawElements(GetType(mode), &count, GetType(type), indices, drawCount);
	}

	void OGLRendererAPI::DrawElements(const RendererEnum mode, const int count, const RendererEnum type, const void* indices)
	{
		glDrawElements(GetType(mode), count, GetType(type), indices);
	}

	void OGLRendererAPI::DrawArrays(int from, int count)
	{
		glDrawArrays(GL_TRIANGLES, from, count);
	}

	GLenum OGLRendererAPI::GetType(const RendererEnum& bufferType)
	{
		switch (bufferType)
		{
			case RendererEnum::ARRAY_BUFFER: return GL_ARRAY_BUFFER;
			case RendererEnum::ELEMENT_ARRAY_BUFFER: return GL_ELEMENT_ARRAY_BUFFER;
			case RendererEnum::FLOAT: return GL_FLOAT;
			case RendererEnum::UFLOAT: return GL_BYTE;
			case RendererEnum::BYTE: return GL_BYTE;
			case RendererEnum::UBYTE: return GL_UNSIGNED_BYTE;
			case RendererEnum::INT: return GL_INT;
			case RendererEnum::UINT: return GL_UNSIGNED_INT;
			case RendererEnum::TRIANGLES: return GL_TRIANGLES;
			case RendererEnum::DEPTH_TEST: return GL_DEPTH_TEST;
			case RendererEnum::FACE_CULL: return GL_CULL_FACE;
		}

		return 0;
	}
}
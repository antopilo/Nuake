#include "UniformBuffer.h"
#include <glad/glad.h>

namespace Nuake
{
	UniformBuffer::UniformBuffer(unsigned int size)
	{
		glGenBuffers(1, &RendererID);
		glBindBuffer(GL_UNIFORM_BUFFER, RendererID);
		glBufferData(GL_UNIFORM_BUFFER, size, 0, GL_STATIC_DRAW);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
	}

	void UniformBuffer::Bind()
	{
		glBindBuffer(GL_UNIFORM_BUFFER, RendererID);
	}

	void UniformBuffer::UpdateData(unsigned int size, void* data)
	{
		glBufferSubData(GL_UNIFORM_BUFFER, 0, size, data);
	}

	void UniformBuffer::Push(unsigned int slot)
	{
		glBindBufferBase(GL_UNIFORM_BUFFER, slot, RendererID);
	}

	void UniformBuffer::Unbind()
	{
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
	}
}

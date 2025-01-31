#pragma once

namespace Nuake
{
	class UniformBuffer {
	private:
		unsigned int RendererID;
	public:
		UniformBuffer(unsigned int size);

		void Bind();
		void Unbind();

		void UpdateData(unsigned int size, void* data);
		void Push(unsigned int slot = 0);
	};
}
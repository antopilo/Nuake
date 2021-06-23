#pragma once


class UniformBuffer {
private:
	unsigned int RendererID;
public:

	UniformBuffer();



	void Bind();
	void Unbind();

};
#pragma once
#include "src/Core/Core.h"
#include "src/Core/Maths.h"

namespace Nuake {
	class RenderBuffer 
	{
	public:
		RenderBuffer(Vector2 size);
		~RenderBuffer();

		void Bind();
		void Unbind();

		unsigned int GetRenderID() const { return m_RenderBuffer; }
	private:
		unsigned int m_RenderBuffer;
	};
}
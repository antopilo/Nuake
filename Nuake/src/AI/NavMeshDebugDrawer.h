#pragma once

#include <DebugDraw.h>

namespace Nuake {


	class GLCheckerTexture
	{
	private:
		unsigned int m_texId;

	public:
		GLCheckerTexture();
		~GLCheckerTexture();

		void bind();
	};


	class NavMeshDebugDrawer : public duDebugDraw
	{
	private:
		//GLCheckerTexture m_Texture;

	public:
		virtual void depthMask(bool state);
		virtual void texture(bool state);

		virtual void begin(duDebugDrawPrimitives prim, float size = 1.0f);

		virtual void vertex(const float* pos, unsigned int color);
		virtual void vertex(const float x, const float y, const float z, unsigned int color);
		virtual void vertex(const float* pos, unsigned int color, const float* uv);
		virtual void vertex(const float x, const float y, const float z, unsigned int color, const float u, const float v);
			
		virtual void end();
	};
}
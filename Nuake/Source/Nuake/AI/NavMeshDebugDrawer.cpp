#include "NavMeshDebugDrawer.h"
#include <glad/glad.h>


namespace Nuake {

	GLCheckerTexture::GLCheckerTexture()
	{
		if (m_texId != 0)
			glDeleteTextures(1, &m_texId);
	}

	void GLCheckerTexture::bind()
	{
		if (m_texId == 0)
		{
			// Create checker pattern.
			const unsigned int col0 = duRGBA(215, 215, 215, 255);
			const unsigned int col1 = duRGBA(255, 255, 255, 255);
			static const int TSIZE = 64;
			unsigned int data[TSIZE * TSIZE];

			glGenTextures(1, &m_texId);
			glBindTexture(GL_TEXTURE_2D, m_texId);

			int level = 0;
			int size = TSIZE;
			while (size > 0)
			{
				for (int y = 0; y < size; ++y)
					for (int x = 0; x < size; ++x)
						data[x + y * size] = (x == 0 || y == 0) ? col0 : col1;
				glTexImage2D(GL_TEXTURE_2D, level, GL_RGBA, size, size, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
				size /= 2;
				level++;
			}

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		}
		else
		{
			glBindTexture(GL_TEXTURE_2D, m_texId);
		}
	}

	GLCheckerTexture::~GLCheckerTexture()
	{
		if (m_texId != 0)
		{
			glDeleteTextures(1, &m_texId);
		}
	}

	void NavMeshDebugDrawer::depthMask(bool state)
	{
		glDepthMask(state ? GL_TRUE : GL_FALSE);
	}

	void NavMeshDebugDrawer::texture(bool state)
	{
		if (state)
		{
			glEnable(GL_TEXTURE_2D);
			//m_Texture.bind();
		}
		else
		{
			glDisable(GL_TEXTURE_2D);
		}
	}

	void NavMeshDebugDrawer::begin(duDebugDrawPrimitives prim, float size)
	{
		switch (prim)
		{
		case DU_DRAW_POINTS:
			glPointSize(size);
			glBegin(GL_POINTS);
			break;
		case DU_DRAW_LINES:
			glLineWidth(size);
			glBegin(GL_LINES);
			break;
		case DU_DRAW_TRIS:
			glBegin(GL_TRIANGLES);
			break;
		case DU_DRAW_QUADS:
			glBegin(GL_QUADS);
			break;
		};
	}

	void NavMeshDebugDrawer::vertex(const float* pos, unsigned int color)
	{
		glColor4ubv((GLubyte*)&color);
		glVertex3fv(pos);
	}

	void NavMeshDebugDrawer::vertex(const float x, const float y, const float z, unsigned int color)
	{
		glColor4ubv((GLubyte*)&color);
		glVertex3f(x, y, z);
	}

	void NavMeshDebugDrawer::vertex(const float* pos, unsigned int color, const float* uv)
	{
		glColor4ubv((GLubyte*)&color);
		glTexCoord2fv(uv);
		glVertex3fv(pos);
	}

	void NavMeshDebugDrawer::vertex(const float x, const float y, const float z, unsigned int color, const float u, const float v)
	{
		glColor4ubv((GLubyte*)&color);
		glTexCoord2f(u, v);
		glVertex3f(x, y, z);
	}

	void NavMeshDebugDrawer::end()
	{
		glEnd();
		glLineWidth(1.0f);
		glPointSize(1.0f);
	}
}

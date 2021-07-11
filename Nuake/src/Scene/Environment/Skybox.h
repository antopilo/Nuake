#pragma once
#include "src/Core/Maths.h"
#include "src/Rendering/Textures/HDR.h"

namespace Nuake
{
	class CubemapTexture;

	class Skybox {
	public:
		Skybox();
		void CreateHDRCubemap();

		void Draw(glm::mat4 projection, glm::mat4 view);
		void Push();
		
	private:
		unsigned int VBO;
		unsigned int VAO;

		// Always the same buffer.
		// TODO: Use primitive.
		static glm::vec3 vertices[];

		CubemapTexture* m_Texture;
		HDRTexture* m_Hdr;
	};
}

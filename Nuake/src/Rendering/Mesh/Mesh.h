#pragma once
#include <vector>
#include "src/Rendering/Textures/Texture.h"
#include "src/Rendering/Vertex.h"
#include <glm\ext\matrix_float4x4.hpp>
#include "src/Rendering/Textures/Material.h"
#include "src/Core/Core.h"

namespace Nuake
{
	class Mesh
	{
	public:
		std::vector<unsigned int> m_Indices;
		std::vector<Vertex> m_Vertices;
		Ref<Material> m_Material;

		Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, Ref<Material> material);

		void Draw(bool bindMaterial = true);
		void DebugDraw();

	private:
		unsigned int VAO, VBO, EBO;
		void setupMesh();
	};
}

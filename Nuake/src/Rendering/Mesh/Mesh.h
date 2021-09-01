#pragma once
#include "src/Core/Core.h"
#include "src/Core/Maths.h"
#include "src/Rendering/Shaders/Shader.h"
#include "src/Rendering/Textures/Material.h"
#include "src/Rendering/Vertex.h"

#include "src/Rendering/Buffers/VertexBuffer.h"
#include "src/Rendering/Buffers/VertexArray.h"
#include "src/Rendering/Buffers/VertexBufferLayout.h"
#include "src/Rendering/AABB.h"

namespace Nuake
{
	class Mesh
	{
	public:
		std::vector<unsigned int> m_Indices;
		std::vector<Vertex> m_Vertices;
		Ref<Material> m_Material;
		
		Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, Ref<Material> material);

		void Draw(Ref<Shader> shader, bool bindMaterial = true);
		void DebugDraw();
		
		inline AABB GetAABB() const { return m_AABB; }
	private:
		VertexBuffer* m_VertexBuffer;
		VertexArray* m_VertexArray;
		VertexBuffer* m_ElementBuffer;
		
		unsigned int VAO, VBO, EBO;
		void SetupMesh();
		
		AABB m_AABB;
		void CalculateAABB();
	};
}

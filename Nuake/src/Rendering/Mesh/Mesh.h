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
#include <src/Resource/Serializable.h>

namespace Nuake
{
	class Mesh
	{
	public:
		std::vector<unsigned int> m_Indices;
		std::vector<Vertex> m_Vertices;
		Ref<Material> m_Material;
		
		Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, Ref<Material> material);

		void Draw(Shader* shader, bool bindMaterial = true);
		void DebugDraw();

		inline AABB GetAABB() const { return m_AABB; }

		json Serialize()
		{
			BEGIN_SERIALIZE();

			j["Material"] = m_Material->Serialize();
			j["Indices"] = m_Indices;

			for (unsigned int i = 0; i < m_Vertices.size(); i++)
			{
				j["Vertices"][i]["Position"] = SERIALIZE_VEC3(m_Vertices[i].position);
				j["Vertices"][i]["Normal"] = SERIALIZE_VEC3(m_Vertices[i].normal);
				j["Vertices"][i]["UV"] = SERIALIZE_VEC2(m_Vertices[i].uv);
				j["Vertices"][i]["Tangent"] = SERIALIZE_VEC3(m_Vertices[i].tangent);
				j["Vertices"][i]["Bitangent"] = SERIALIZE_VEC3(m_Vertices[i].bitangent);
			}
			
			END_SERIALIZE();
		}

		bool Deserialize(std::string str)
		{
			BEGIN_DESERIALIZE();
			

			return true;
		}

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

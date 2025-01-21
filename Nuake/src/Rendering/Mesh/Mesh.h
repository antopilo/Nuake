#pragma once
#include "src/Core/Core.h"
#include "src/Rendering/AABB.h"
#include "src/Resource/Resource.h"
#include "src/Resource/Serializable.h"
#include "src/Rendering/Vulkan/VkMesh.h"
#include "src/Resource/RID.h"

namespace Nuake
{
	class VertexBuffer;
	class VertexArray;
	class Material;
	struct Vertex;
	class Shader;

	class Mesh : ISerializable, Resource
	{
	public:
		Mesh();
		~Mesh();
		
		// This sets the data of the mesh without uploading it to the GPU.
		// Useful for async asset baking
		void SetData(std::vector<Vertex>& vertices, std::vector<uint32_t>& indices);
		
		// This sets the data and uploads it to the GPU
		void AddSurface(std::vector<Vertex> vertices, std::vector<uint32_t> indices);
		std::vector<Vertex>& GetVertices();
		std::vector<uint32_t>& GetIndices();

		Ref<Material> GetMaterial() const;
		void SetMaterial(Ref<Material> material);

		void Bind() const;
		void Draw(Shader* shader, bool bindMaterial = true);
		void DebugDraw();

		inline AABB GetAABB() const { return m_AABB; }

		json Serialize() override;
		bool Deserialize(const json& j) override;

		Ref<VkMesh> GetVkMesh() 
		{
			return m_Mesh;
		}

		RID MaterialResource;
	private:

		Ref<Material> m_Material = nullptr;
		std::vector<uint32_t> m_Indices;
		std::vector<Vertex> m_Vertices;

		uint32_t m_IndicesCount;
		uint32_t m_VerticesCount;

		Ref<VkMesh> m_Mesh;
		//Scope<VertexBuffer> m_VertexBuffer;
		//Scope<VertexArray> m_VertexArray;
		//Scope<VertexBuffer> m_ElementBuffer;

		void SetupMesh();

		AABB m_AABB;
		void CalculateAABB();
	};
}
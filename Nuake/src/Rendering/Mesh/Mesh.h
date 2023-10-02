#pragma once
#include "src/Core/Core.h"
#include "src/Rendering/AABB.h"
#include "src/Resource/Resource.h"
#include "src/Resource/Serializable.h"

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

	private:
		Ref<Material> m_Material = nullptr;
		std::vector<uint32_t> m_Indices;
		std::vector<Vertex> m_Vertices;

		uint32_t m_IndicesCount;
		uint32_t m_VerticesCount;

		Scope<VertexBuffer> m_VertexBuffer;
		Scope<VertexArray> m_VertexArray;
		Scope<VertexBuffer> m_ElementBuffer;

		void SetupMesh();

		AABB m_AABB;
		void CalculateAABB();
	};
}
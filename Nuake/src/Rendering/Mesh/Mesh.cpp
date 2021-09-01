#include "Mesh.h"
#include "src/Rendering/Renderer.h"
#include "src/Rendering/Textures/MaterialManager.h"

namespace Nuake
{
    Mesh::Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, Ref<Material> material)
    {
        this->m_Vertices = vertices;
        this->m_Indices = indices;
        this->m_Material = material;

        MaterialManager::Get()->RegisterMaterial(material);

        SetupMesh();
        CalculateAABB();
    }
	
	void Mesh::CalculateAABB()
	{
        float minX = 0.0f;
        float minY = 0.0f;
        float minZ = 0.0f;
        float maxX = 0.0f;
        float maxY = 0.0f;
        float maxZ = 0.0f;
		for (Vertex& v : m_Vertices)
		{
			minX = v.position.x < minX ? v.position.x : minX;
			minY = v.position.y < minY ? v.position.y : minY;
			minZ = v.position.z < minZ ? v.position.z : minZ;
			maxX = v.position.x > maxX ? v.position.x : maxX;
			maxY = v.position.y > maxY ? v.position.y : maxY;
			maxZ = v.position.z > maxZ ? v.position.z : maxZ;
		}
        this->m_AABB = { Vector3(minX, minY, minZ), Vector3(maxX, maxY, maxZ) };
	}

    void Mesh::SetupMesh()
    {
        m_VertexArray = new VertexArray();
        m_VertexArray->Bind();
        m_VertexBuffer = new VertexBuffer(m_Vertices.data(), m_Vertices.size() * sizeof(Vertex));
        m_ElementBuffer = new VertexBuffer(m_Indices.data(), m_Indices.size() * sizeof(unsigned int), RendererEnum::ELEMENT_ARRAY_BUFFER);

        VertexBufferLayout bufferLayout = VertexBufferLayout();
        bufferLayout.Push<float>(3); // Position
        bufferLayout.Push<float>(2); // UV
        bufferLayout.Push<float>(3); // Normal
        bufferLayout.Push<float>(3); // Tangent
        bufferLayout.Push<float>(3); // Bitangent
        bufferLayout.Push<float>(1); // Texture

        m_VertexArray->AddBuffer(*m_VertexBuffer, bufferLayout);
        m_VertexArray->Unbind();
    }

    void Mesh::Draw(Ref<Shader> shader, bool bindMaterial)
    {
        if (bindMaterial)
            m_Material->Bind(shader);

        m_VertexArray->Bind();
        RenderCommand::DrawElements(RendererEnum::TRIANGLES, m_Indices.size(), RendererEnum::UINT, 0);
    }

    void Mesh::DebugDraw()
    {
        Renderer::m_DebugShader->Bind();
        Renderer::m_DebugShader->SetUniform4f("u_Color", 1.0f, 0.0f, 0.0f, 1.f);

        m_VertexArray->Bind();
        RenderCommand::DrawElements(RendererEnum::TRIANGLES, m_Indices.size(), RendererEnum::UINT, 0);
    }
}

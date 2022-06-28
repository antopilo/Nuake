#include "Mesh.h"

#include "src/Core/Maths.h"
#include "src/Rendering/Textures/Material.h"
#include "src/Rendering/Textures/MaterialManager.h"
#include "src/Rendering/Renderer.h"
#include "src/Rendering/Shaders/Shader.h"
#include "src/Rendering/Vertex.h"
#include "src/Rendering/Buffers/VertexBuffer.h"
#include "src/Rendering/Buffers/VertexArray.h"
#include "src/Rendering/Buffers/VertexBufferLayout.h"

namespace Nuake
{
    Mesh::Mesh() {}
    Mesh::~Mesh() {}

    void Mesh::AddSurface(std::vector<Vertex> vertices, std::vector<uint32_t> indices)
    {
        this->m_Vertices = vertices;
        this->m_Indices = indices;

        SetupMesh();
        CalculateAABB();

        if (m_Material == nullptr)
        {
            m_Material = MaterialManager::Get()->GetMaterial("default");
        }
    }

    std::vector<Vertex>& Mesh::GetVertices()
    {
        return m_Vertices;
    }

    std::vector<uint32_t>& Mesh::GetIndices()
    {
        return m_Indices;
    }

    Ref<Material> Mesh::GetMaterial() inline const
    {
        return m_Material;
    }

    void Mesh::SetMaterial(Ref<Material> material)
    {
        m_Material = material;
        MaterialManager::Get()->RegisterMaterial(material);
    }
	
	void Mesh::CalculateAABB()
	{
        float minX = 0.0f;
        float minY = 0.0f;
        float minZ = 0.0f;
        float maxX = 0.0f;
        float maxY = 0.0f;
        float maxZ = 0.0f;

		for (const Vertex& v : m_Vertices)
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
        m_VertexArray = CreateScope<VertexArray>();
        m_VertexArray->Bind();
        m_VertexBuffer = CreateScope<VertexBuffer>(m_Vertices.data(), m_Vertices.size() * sizeof(Vertex));
        m_ElementBuffer = CreateScope<VertexBuffer>(m_Indices.data(), m_Indices.size() * sizeof(unsigned int), RendererEnum::ELEMENT_ARRAY_BUFFER);

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

    void Mesh::Draw(Shader* shader, bool bindMaterial)
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

    json Mesh::Serialize()
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

    bool Mesh::Deserialize(const std::string& str)
    {
        BEGIN_DESERIALIZE();


        return true;
    }
}

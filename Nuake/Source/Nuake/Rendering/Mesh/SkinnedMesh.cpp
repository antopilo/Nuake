#include "SkinnedMesh.h"

#include "Nuake/Core/Maths.h"
#include "Nuake/Rendering/Textures/Material.h"
#include "Nuake/Rendering/Textures/MaterialManager.h"
#include "Nuake/Rendering/Renderer.h"
#include "Nuake/Rendering/Shaders/Shader.h"

#include "Nuake/Rendering/Vertex.h"
#include "Nuake/Rendering/Buffers/VertexBuffer.h"
#include "Nuake/Rendering/Buffers/VertexArray.h"
#include "Nuake/Rendering/Buffers/VertexBufferLayout.h"


#include <future>

namespace Nuake
{
    void SkinnedMesh::AddSurface(std::vector<SkinnedVertex> vertices, std::vector<uint32_t> indices, std::vector<Bone> bones)
    {
        m_Vertices = vertices;
        m_Indices = indices;

        SetupMesh();
        CalculateAABB();

        if (m_Material == nullptr)
        {
            m_Material = MaterialManager::Get()->GetMaterial("default");
        }
    }

    std::vector<SkinnedVertex>& SkinnedMesh::GetVertices()
    {
        return m_Vertices;
    }

    std::vector<uint32_t>& SkinnedMesh::GetIndices()
    {
        return m_Indices;
    }

    Ref<Material> SkinnedMesh::GetMaterial() const
    {
        return m_Material;
    }

    void SkinnedMesh::SetMaterial(Ref<Material> material)
    {
        m_Material = material;
        MaterialManager::Get()->RegisterMaterial(material);
    }

    void SkinnedMesh::CalculateAABB()
    {
        float minX = 0.0f;
        float minY = 0.0f;
        float minZ = 0.0f;
        float maxX = 0.0f;
        float maxY = 0.0f;
        float maxZ = 0.0f;

        for (const SkinnedVertex& v : m_Vertices)
        {
            minX = v.position.x < minX ? v.position.x : minX;
            minY = v.position.y < minY ? v.position.y : minY;
            minZ = v.position.z < minZ ? v.position.z : minZ;
            maxX = v.position.x > maxX ? v.position.x : maxX;
            maxY = v.position.y > maxY ? v.position.y : maxY;
            maxZ = v.position.z > maxZ ? v.position.z : maxZ;
        }

        m_AABB = AABB(Vector3(minX, minY, minZ), Vector3(maxX, maxY, maxZ));
    }

    void SkinnedMesh::SetupMesh()
    {
        m_VertexArray = CreateScope<VertexArray>();
        m_VertexArray->Bind();
        m_VertexBuffer = CreateScope<VertexBuffer>(m_Vertices.data(), m_Vertices.size() * sizeof(SkinnedVertex));
        m_ElementBuffer = CreateScope<VertexBuffer>(m_Indices.data(), m_Indices.size() * sizeof(unsigned int), RendererEnum::ELEMENT_ARRAY_BUFFER);

        VertexBufferLayout bufferLayout = VertexBufferLayout();
        bufferLayout.Push<float>(3); // Position
        bufferLayout.Push<float>(2); // UV
        bufferLayout.Push<float>(3); // Normal
        bufferLayout.Push<float>(3); // Tangent
        bufferLayout.Push<float>(3); // Bitangent
        bufferLayout.Push<int>(4);   // BoneIds
        bufferLayout.Push<float>(4); // Weights

        m_VertexArray->AddBuffer(*m_VertexBuffer, bufferLayout);
        m_VertexArray->Unbind();
    }

    void SkinnedMesh::Bind() const
    {
        m_VertexArray->Bind();
    }

    void SkinnedMesh::Draw(Shader* shader, bool bindMaterial)
    {
        if (bindMaterial)
            m_Material->Bind(shader);

        m_VertexArray->Bind();
        RenderCommand::DrawElements(RendererEnum::TRIANGLES, (int)m_Indices.size(), RendererEnum::UINT, 0);
    }

    void SkinnedMesh::DebugDraw()
    {
        Renderer::m_DebugShader->Bind();
        Renderer::m_DebugShader->SetUniform("u_Color", 1.0f, 0.0f, 0.0f, 1.f);

        m_VertexArray->Bind();
        RenderCommand::DrawElements(RendererEnum::TRIANGLES, (int)m_Indices.size(), RendererEnum::UINT, 0);
    }

    json SkinnedMesh::Serialize()
    {
        BEGIN_SERIALIZE();

        j["Material"] = m_Material->Serialize();
        j["Indices"] = m_Indices;

        json v;
        for (uint32_t i = 0; i < m_Vertices.size(); i++)
        {
            v["Position"]["x"] = m_Vertices[i].position.x;
            v["Position"]["y"] = m_Vertices[i].position.y;
            v["Position"]["z"] = m_Vertices[i].position.z;

            v["UV"]["x"] = m_Vertices[i].uv.x;
            v["UV"]["y"] = m_Vertices[i].uv.y;

            v["Normal"]["x"] = m_Vertices[i].normal.x;
            v["Normal"]["y"] = m_Vertices[i].normal.y;
            v["Normal"]["z"] = m_Vertices[i].normal.z;

            v["Tangent"]["x"] = m_Vertices[i].tangent.x;
            v["Tangent"]["y"] = m_Vertices[i].tangent.y;
            v["Tangent"]["z"] = m_Vertices[i].tangent.z;

            v["Bitangent"]["x"] = m_Vertices[i].bitangent.x;
            v["Bitangent"]["y"] = m_Vertices[i].bitangent.y;
            v["Bitangent"]["z"] = m_Vertices[i].bitangent.z;

            for (uint32_t b = 0; b < MAX_BONE_INFLUENCE; b++)
            {
                v["Weight"][b] = m_Vertices[i].weights[b];
                v["BoneIDs"][b] = m_Vertices[i].boneIDs[b];
            }

            j["Vertices"][i] = v;
        }


        END_SERIALIZE();
    }

    bool SkinnedMesh::Deserialize(const json& j)
    {
        m_Material = CreateRef<Material>();
        m_Material->Deserialize(j["Material"]);

        m_Indices.reserve(j["Indices"].size());
        for (auto& i : j["Indices"])
        {
            m_Indices.push_back(i);
        }

        std::vector<SkinnedVertex> vertices;

        std::async(std::launch::async, [&]()
            {
                for (auto& v : j["Vertices"])
                {
                    SkinnedVertex vertex;
                    try {
                        DESERIALIZE_VEC2(v["UV"], vertex.uv)
                    }
                    catch (std::exception& /*e*/) {
                        vertex.uv = { 0.0, 0.0 };
                    }
                    DESERIALIZE_VEC3(v["Position"], vertex.position)
                    DESERIALIZE_VEC3(v["Normal"], vertex.normal)
                    DESERIALIZE_VEC3(v["Tangent"], vertex.tangent)
                    DESERIALIZE_VEC3(v["Bitangent"], vertex.bitangent)

                    for (uint32_t i = 0; i < MAX_BONE_INFLUENCE; i++)
                    {
                        vertex.weights[i] = v["Weight"][i];
                        vertex.boneIDs[i] = v["boneIDs"][i];
                    }

                    vertices.push_back(vertex);
                }
            }
        );

        m_Vertices = vertices;

        SetupMesh();
        return true;
    }
}

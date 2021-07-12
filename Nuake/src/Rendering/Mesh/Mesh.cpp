#include "Mesh.h"
#include <GL\glew.h>

#include "src/Rendering/Renderer.h"
#include "src/Core/MaterialManager.h"

namespace Nuake
{
    Mesh::Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, Ref<Material> material)
    {
        this->m_Vertices = vertices;
        this->m_Indices = indices;
        this->m_Material = material;

        MaterialManager::Get()->RegisterMaterial(material);

        setupMesh();
    }

    void Mesh::setupMesh()
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

    void Mesh::Draw(bool drawMaterial)
    {
        if (drawMaterial)
            m_Material->Bind();

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

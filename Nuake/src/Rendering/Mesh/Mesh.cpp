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
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);

        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);

        glBufferData(GL_ARRAY_BUFFER, m_Vertices.size() * sizeof(Vertex), &m_Vertices[0], GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_Indices.size() * sizeof(unsigned int), &m_Indices[0], GL_STATIC_DRAW);

        // vertex positions
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
        glEnableVertexAttribArray(0);

        // UV
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(sizeof(GL_FLOAT) * 3));
        glEnableVertexAttribArray(1);

        // Normal
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(sizeof(GL_FLOAT) * 5));
        glEnableVertexAttribArray(2);

        // Tangent
        glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(sizeof(GL_FLOAT) * 8));
        glEnableVertexAttribArray(3);

        // Bi tangent
        glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(sizeof(GL_FLOAT) * 11));
        glEnableVertexAttribArray(4);

        // Texture
        glVertexAttribPointer(5, 1, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(sizeof(GL_FLOAT) * 14));
        glEnableVertexAttribArray(5);


        glBindVertexArray(0);
    }

    void Mesh::Draw(bool drawMaterial)
    {
        if (drawMaterial)
            m_Material->Bind();
        // draw mesh
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, m_Indices.size(), GL_UNSIGNED_INT, 0);
        //glBindVertexArray(0);
    }

    void Mesh::DebugDraw()
    {
        Renderer::m_DebugShader->Bind();
        Renderer::m_DebugShader->SetUniform4f("u_Color", 1.0f, 0.0f, 0.0f, 1.f);
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, m_Indices.size(), GL_UNSIGNED_INT, 0);
    }
}

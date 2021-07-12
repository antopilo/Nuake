#include "MeshComponent.h"
#include <GL\glew.h>
#include "src/Rendering/Renderer.h"
#include "src/Rendering/Vertex.h"
#include "src/Core/MaterialManager.h"
#include <imgui\imgui.h>

namespace Nuake {
    Vertex vertices[] = {
            Vertex{ glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec2(1.0f, 0.0f), glm::vec3(0, 0, -1), glm::vec3(0, 1, 0), glm::vec3(-1, 0, 0), 1.0f  },
            Vertex{ glm::vec3(0.5f, -0.5f, -0.5f),  glm::vec2(1.0f, 1.0f), glm::vec3(0, 0, -1), glm::vec3(0, 1, 0), glm::vec3(-1, 0, 0), 1.0f  },
            Vertex{ glm::vec3(0.5f,  0.5f, -0.5f),  glm::vec2(0.0f, 1.0f), glm::vec3(0, 0, -1), glm::vec3(0, 1, 0), glm::vec3(-1, 0, 0), 1.0f  },
            Vertex{ glm::vec3(0.5f,  0.5f, -0.5f),  glm::vec2(0.0f, 1.0f), glm::vec3(0, 0, -1), glm::vec3(0, 1, 0), glm::vec3(-1, 0, 0), 1.0f  },
            Vertex{ glm::vec3(-0.5f,  0.5f, -0.5f), glm::vec2(0.0f, 0.0f), glm::vec3(0, 0, -1), glm::vec3(0, 1, 0), glm::vec3(-1, 0, 0), 1.0f  },
            Vertex{ glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec2(1.0f, 0.0f), glm::vec3(0, 0, -1), glm::vec3(0, 1, 0), glm::vec3(-1, 0, 0), 1.0f  },

            Vertex{ glm::vec3(-0.5f, -0.5f,  0.5f), glm::vec2(1.0f, 0.0f), glm::vec3(0, 0, 1), glm::vec3(0, 1, 0), glm::vec3(1, 0, 0), 1.0f  },
            Vertex{ glm::vec3(0.5f, -0.5f,  0.5f),  glm::vec2(1.0f, 1.0f), glm::vec3(0, 0, 1), glm::vec3(0, 1, 0), glm::vec3(1, 0, 0), 1.0f  },
            Vertex{ glm::vec3(0.5f,  0.5f,  0.5f),  glm::vec2(0.0f, 1.0f), glm::vec3(0, 0, 1), glm::vec3(0, 1, 0), glm::vec3(1, 0, 0), 1.0f  },
            Vertex{ glm::vec3(0.5f,  0.5f,  0.5f),  glm::vec2(0.0f, 1.0f), glm::vec3(0, 0, 1), glm::vec3(0, 1, 0), glm::vec3(1, 0, 0), 1.0f  },
            Vertex{ glm::vec3(-0.5f,  0.5f,  0.5f), glm::vec2(0.0f, 0.0f), glm::vec3(0, 0, 1), glm::vec3(0, 1, 0), glm::vec3(1, 0, 0), 1.0f  },
            Vertex{ glm::vec3(-0.5f, -0.5f,  0.5f), glm::vec2(1.0f, 0.0f), glm::vec3(0, 0, 1), glm::vec3(0, 1, 0), glm::vec3(1, 0, 0), 1.0f  },

            Vertex{ glm::vec3(-0.5f,  0.5f,  0.5f), glm::vec2(1.0f, 0.0f), glm::vec3(-1, 0, 0), glm::vec3(0, 1, 0), glm::vec3(0, 0, 1), 1.0f  },
            Vertex{ glm::vec3(-0.5f,  0.5f, -0.5f), glm::vec2(1.0f, 1.0f), glm::vec3(-1, 0, 0), glm::vec3(0, 1, 0), glm::vec3(0, 0, 1), 1.0f  },
            Vertex{ glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec2(0.0f, 1.0f), glm::vec3(-1, 0, 0), glm::vec3(0, 1, 0), glm::vec3(0, 0, 1), 1.0f  },
            Vertex{ glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec2(0.0f, 1.0f), glm::vec3(-1, 0, 0), glm::vec3(0, 1, 0), glm::vec3(0, 0, 1), 1.0f  },
            Vertex{ glm::vec3(-0.5f, -0.5f,  0.5f), glm::vec2(0.0f, 0.0f), glm::vec3(-1, 0, 0), glm::vec3(0, 1, 0), glm::vec3(0, 0, 1), 1.0f  },
            Vertex{ glm::vec3(-0.5f,  0.5f,  0.5f), glm::vec2(1.0f, 0.0f), glm::vec3(-1, 0, 0), glm::vec3(0, 1, 0), glm::vec3(0, 0, 1), 1.0f  },

            Vertex{ glm::vec3(0.5f,  0.5f,  0.5f),  glm::vec2(1.0f, 0.0f), glm::vec3(1, 0, 0), glm::vec3(0, 1, 0), glm::vec3(0, 0, -1), 1.0f  },
            Vertex{ glm::vec3(0.5f,  0.5f, -0.5f),  glm::vec2(1.0f, 1.0f), glm::vec3(1, 0, 0), glm::vec3(0, 1, 0), glm::vec3(0, 0, -1), 1.0f  },
            Vertex{ glm::vec3(0.5f, -0.5f, -0.5f),  glm::vec2(0.0f, 1.0f), glm::vec3(1, 0, 0), glm::vec3(0, 1, 0), glm::vec3(0, 0, -1), 1.0f  },
            Vertex{ glm::vec3(0.5f, -0.5f, -0.5f),  glm::vec2(0.0f, 1.0f), glm::vec3(1, 0, 0), glm::vec3(0, 1, 0), glm::vec3(0, 0, -1), 1.0f  },
            Vertex{ glm::vec3(0.5f, -0.5f,  0.5f),  glm::vec2(0.0f, 0.0f), glm::vec3(1, 0, 0), glm::vec3(0, 1, 0), glm::vec3(0, 0, -1), 1.0f  },
            Vertex{ glm::vec3(0.5f,  0.5f,  0.5f),  glm::vec2(1.0f, 0.0f), glm::vec3(1, 0, 0), glm::vec3(0, 1, 0), glm::vec3(0, 0, -1), 1.0f  },

            Vertex{ glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec2(0.0f, 1.0f), glm::vec3(0, -1, 0), glm::vec3(1, 0, 0), glm::vec3(0, 0, 1), 1.0f  },
            Vertex{ glm::vec3(0.5f, -0.5f, -0.5f),  glm::vec2(1.0f, 1.0f), glm::vec3(0, -1, 0), glm::vec3(1, 0, 0), glm::vec3(0, 0, 1), 1.0f  },
            Vertex{ glm::vec3(0.5f, -0.5f,  0.5f),  glm::vec2(1.0f, 0.0f), glm::vec3(0, -1, 0), glm::vec3(1, 0, 0), glm::vec3(0, 0, 1), 1.0f  },
            Vertex{ glm::vec3(0.5f, -0.5f,  0.5f),  glm::vec2(1.0f, 0.0f), glm::vec3(0, -1, 0), glm::vec3(1, 0, 0), glm::vec3(0, 0, 1), 1.0f  },
            Vertex{ glm::vec3(-0.5f, -0.5f,  0.5f), glm::vec2(0.0f, 0.0f), glm::vec3(0, -1, 0), glm::vec3(1, 0, 0), glm::vec3(0, 0, 1), 1.0f  },
            Vertex{ glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec2(0.0f, 1.0f), glm::vec3(0, -1, 0), glm::vec3(1, 0, 0), glm::vec3(0, 0, 1), 1.0f  },

            Vertex{ glm::vec3(-0.5f,  0.5f, -0.5f), glm::vec2(0.0f, 1.0f), glm::vec3(0, 1, 0), glm::vec3(1, 0, 0), glm::vec3(0, 0, 1), 1.0f  },
            Vertex{ glm::vec3(0.5f,  0.5f, -0.5f),  glm::vec2(1.0f, 1.0f), glm::vec3(0, 1, 0), glm::vec3(1, 0, 0), glm::vec3(0, 0, 1), 1.0f  },
            Vertex{ glm::vec3(0.5f,  0.5f,  0.5f),  glm::vec2(1.0f, 0.0f), glm::vec3(0, 1, 0), glm::vec3(1, 0, 0), glm::vec3(0, 0, 1), 1.0f  },
            Vertex{ glm::vec3(0.5f,  0.5f,  0.5f),  glm::vec2(1.0f, 0.0f), glm::vec3(0, 1, 0), glm::vec3(1, 0, 0), glm::vec3(0, 0, 1), 1.0f  },
            Vertex{ glm::vec3(-0.5f,  0.5f,  0.5f), glm::vec2(0.0f, 0.0f), glm::vec3(0, 1, 0), glm::vec3(1, 0, 0), glm::vec3(0, 0, 1), 1.0f  },
            Vertex{ glm::vec3(-0.5f,  0.5f, -0.5f), glm::vec2(0.0f, 1.0f), glm::vec3(0, 1, 0), glm::vec3(1, 0, 0), glm::vec3(0, 0, 1), 1.0f  },
    };

    void MeshComponent::LoadModel(const std::string path) {
        //Assimp::Importer import;
        //const aiScene* scene = import.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);
        //
        //if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
        //{
        //    printf("ERROR::ASSIMP::");
        //    return;
        //}
        //
        //ProcessNode(scene->mRootNode, scene);
    }

    unsigned int sphereVAO = 0;
    unsigned int indexCount;
    

    //void MeshComponent::ProcessNode(aiNode * node, const aiScene* scene)
    //{
        /// process all the node's meshes (if any)
        //or (unsigned int i = 0; i < node->mNumMeshes; i++)
        //
        //   aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        //   meshes.push_back(ProcessNode(mesh, scene));
        //
        /// then do the same for each of its children
        //or (unsigned int i = 0; i < node->mNumChildren; i++)
        //
        //   ProcessNode(node->mChildren[i], scene);
        //
    //}

    MeshComponent::MeshComponent() {
        //BuildTangents();

        // Setup buffers
        glGenVertexArrays(1, &VAO);
        glBindVertexArray(VAO);

        glGenBuffers(1, &VBO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);

        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        // Position
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

        // Bitangent
        glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(sizeof(GL_FLOAT) * 11));
        glEnableVertexAttribArray(4);

        // Texture
        glVertexAttribPointer(5, 1, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(sizeof(GL_FLOAT) * 14));
        glEnableVertexAttribArray(5);

        //m_Material = MaterialManager::Get()->LoadMaterial("Planks");

    } 

    void MeshComponent::BuildTangents()
    {
        for (int i = 0; i < 36; i += 3) {
            glm::vec3 pos1 = vertices[i].position;
            glm::vec3 pos2 = vertices[i + 1].position;
            glm::vec3 pos3 = vertices[i + 2].position;

            glm::vec2 uv1 = vertices[i].uv;
            glm::vec2 uv2 = vertices[i + 1].uv;
            glm::vec2 uv3 = vertices[i + 2].uv;

            glm::vec3 edge1 = pos2 - pos1;
            glm::vec3 edge2 = pos3 - pos1;
            glm::vec2 deltaUV1 = uv2 - uv1;
            glm::vec2 deltaUV2 = uv3 - uv1;

            float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

            for (int j = 0; i < 3; i++)
            {
                vertices[i + j].tangent.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
                vertices[i + j].tangent.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
                vertices[i + j].tangent.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);

                vertices[i + j].bitangent.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
                vertices[i + j].bitangent.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
                vertices[i + j].bitangent.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);
            }
        }
    }

    void MeshComponent::SetMaterial(const std::string materialName) 
    {
        m_Material = MaterialManager::Get()->LoadMaterial(materialName);
    }

    void MeshComponent::Draw(glm::mat4 projection, glm::mat4 view, glm::mat4 transform) {

        Renderer::m_Shader->SetUniformMat4f("u_Model", transform);

        m_Material->Bind();

        //RenderSphere();
       glBindVertexArray(VAO);
       glDrawArrays(GL_TRIANGLES, 0, 36);
    }

    void MeshComponent::DrawEditor() {
        int choice = 0;
        ImGui::Combo("Material", (int*)&choice, "Marble\0Copper\0Gold\0Paving\0Planks\0Default Material");
    }
}


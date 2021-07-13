#include "MeshComponent.h"
#include "src/Rendering/Textures/Material.h"
#include "src/Rendering/Renderer.h"
#include "src/Rendering/Textures/TextureManager.h"

namespace Nuake {
    void MeshComponent::Draw()
    {
        for (auto m : meshes)
            m->Draw();
    }

    void MeshComponent::LoadModel()
    {
        this->meshes.clear();
        Assimp::Importer import;
        import.SetPropertyFloat("PP_GSN_MAX_SMOOTHING_ANGLE", 90);
        const aiScene* scene = import.ReadFile(FileSystem::Root + ModelPath, aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_CalcTangentSpace);

        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
        {
            Logger::Log("ASSIMP! Failed to load model" + std::string(import.GetErrorString()), CRITICAL);
            return;
        }

        ProcessNode(scene->mRootNode, scene);
    }

    void MeshComponent::ProcessNode(aiNode* node, const aiScene* scene)
    {
        // process all the node's meshes (if any)
        for (unsigned int i = 0; i < node->mNumMeshes; i++)
        {
            aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
            meshes.push_back(ProcessMesh(mesh, scene));
        }
        // then do the same for each of its children
        for (unsigned int i = 0; i < node->mNumChildren; i++)
        {
            ProcessNode(node->mChildren[i], scene);
        }
    }

    Ref<Mesh> MeshComponent::ProcessMesh(aiMesh* mesh, const aiScene* scene)
    {
        std::vector<Vertex> vertices;
        std::vector<unsigned int> indices;
        std::vector<Texture> textures;

        for (unsigned int i = 0; i < mesh->mNumVertices; i++)
        {
            Vertex vertex;
            vertex.texture = 1.0f;

            glm::vec3 vector;
            vector.x = mesh->mVertices[i].x;
            vector.y = mesh->mVertices[i].y;
            vector.z = mesh->mVertices[i].z;
            vertex.position = vector;

            vector.x = mesh->mNormals[i].x;
            vector.y = mesh->mNormals[i].y;
            vector.z = mesh->mNormals[i].z;
            vertex.normal = vector;

            vector.x = mesh->mTangents[i].x;
            vector.y = mesh->mTangents[i].y;
            vector.z = mesh->mTangents[i].z;
            vertex.tangent = vector;

            vector.x = mesh->mBitangents[i].x;
            vector.y = mesh->mBitangents[i].y;
            vector.z = mesh->mBitangents[i].z;
            vertex.bitangent = vector;

            if (mesh->mTextureCoords[0]) // does the mesh contain texture coordinates?
            {
                glm::vec2 vec;
                vec.x = mesh->mTextureCoords[0][i].x;
                vec.y = mesh->mTextureCoords[0][i].y;
                vertex.uv = vec;
            }
            else
                vertex.uv = glm::vec2(0.0f, 0.0f);

            vertices.push_back(vertex);
        }
        // process indices
        for (unsigned int i = 0; i < mesh->mNumFaces; i++)
        {
            aiFace face = mesh->mFaces[i];
            for (unsigned int j = 0; j < face.mNumIndices; j++)
                indices.push_back(face.mIndices[j]);
        }
        // process material
        if (mesh->mMaterialIndex >= 0)
        {
            aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
            aiString str;
            std::string directory = FileSystem::Root + this->ModelPath + "/../";
            material->GetTexture(aiTextureType_DIFFUSE, 0, &str);
            Ref<Material> newMaterial = CreateRef<Material>(TextureManager::Get()->GetTexture(directory + str.C_Str()));

            //material->GetTexture(aiTextureType_NORMALS, 0, &str);
            //newMaterial->SetNormal(TextureManager::Get()->GetTexture(directory + str.C_Str()));
            //
            //material->GetTexture(aiTextureType_METALNESS, 0, &str);
            //newMaterial->SetMetalness(TextureManager::Get()->GetTexture(directory + str.C_Str()));
            //
            //material->GetTexture(aiTextureType_DIFFUSE_ROUGHNESS, 0, &str);
            //newMaterial->SetRoughness(TextureManager::Get()->GetTexture(directory + str.C_Str()));

            //material->GetTexture(aiTextureType_DISPLACEMENT, 0, &str);
            //newMaterial->SetDisplacement(TextureManager::Get()->GetTexture(directory + str.C_Str()));

            //material->GetTexture(aiTextureType_AMBIENT_OCCLUSION, 0, &str);
            //newMaterial->SetAO(TextureManager::Get()->GetTexture(directory + str.C_Str()));

            return CreateRef<Mesh>(vertices, indices, newMaterial);
        }
    }

    std::vector<Texture*> MeshComponent::LoadMaterialTextures(aiMaterial* mat, aiTextureType type)
    {
        std::vector<Texture*> textures;
        for (unsigned int i = 0; i < mat->GetTextureCount(type); i++)
        {
            aiString str;
            mat->GetTexture(type, i, &str);
            std::string fixedStr = std::string(str.C_Str());

            Texture* texture = new Texture(directory + fixedStr);
            textures.push_back(texture);
        }
        return textures;
    }
}


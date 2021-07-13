#pragma once
#include <glm\ext\matrix_float4x4.hpp>
#include <vector>
#include "src/Rendering/Mesh/Mesh.h"

#include "assimp/Importer.hpp"
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <string>

namespace Nuake
{
    struct MeshComponent
    {
        std::string ModelPath;

        MeshComponent()
        {
            //loadModel(path);
        }
        void LoadModel();
        void Draw();
        std::vector<Ref<Mesh>> meshes;
        std::string directory;

        void ProcessNode(aiNode* node, const aiScene* scene);
        Ref<Mesh> ProcessMesh(aiMesh* mesh, const aiScene* scene);
        std::vector<Texture*> LoadMaterialTextures(aiMaterial* mat, aiTextureType type);
    };
}

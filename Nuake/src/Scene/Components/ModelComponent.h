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
    class ModelComponent
    {
    public:
        std::string ModelPath;

        ModelComponent()
        {
            //loadModel(path);
        }
        void LoadModel();
        void Draw();
    private:
        std::vector<Mesh> meshes;
        std::string directory;

        void ProcessNode(aiNode* node, const aiScene* scene);
        Mesh ProcessMesh(aiMesh* mesh, const aiScene* scene);
        std::vector<Texture*> LoadMaterialTextures(aiMaterial* mat, aiTextureType type);
    };
}

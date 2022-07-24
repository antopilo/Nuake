#pragma once
#include "src/Core/Core.h"

#include "src/Rendering/Mesh/Mesh.h"
#include "src/Rendering/Textures/Material.h"
#include "src/Rendering/Textures/Texture.h"

#include "src/Rendering/Vertex.h"

#include "assimp/Importer.hpp"
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <bitset>

namespace Nuake
{


	class Model;
	class ModelLoader
	{
	public:
		ModelLoader();
		~ModelLoader();

		Ref<Model> LoadModel(const std::string& path);

	private:
		std::bitset<8> _flags;
		std::string modelDir;
		std::vector<Ref<Mesh>> m_Meshes;

		void ProcessNode(aiNode* node, const aiScene* scene);
		Ref<Mesh> ProcessMesh(aiMesh* node, const aiScene* scene);

		std::vector<Vertex> ProcessVertices(aiMesh* mesh);
		std::vector<uint32_t> ProcessIndices(aiMesh* mesh);
		Ref<Material> ProcessMaterials(const aiScene* scene, aiMesh* mesh);
		Ref<Texture> ProcessTextures(const aiScene* scene, const std::string& path);
	};
}
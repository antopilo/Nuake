#pragma once
#include "src/Core/Core.h"

#include "src/Rendering/Mesh/SkinnedMesh.h"
#include "src/Rendering/Mesh/Mesh.h"
#include "src/Rendering/Textures/Material.h"
#include "src/Rendering/Textures/Texture.h"

#include "src/Rendering/Vertex.h"

#include "assimp/Importer.hpp"
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "src/Rendering/Buffers/VertexArray.h"

namespace Nuake
{
	class Model;
	class SkinnedModel;

	class ModelLoader
	{
	public:
		ModelLoader();
		~ModelLoader();

		Ref<Model> LoadModel(const std::string& path, bool absolute = false);
		Ref<SkinnedModel> LoadSkinnedModel(const std::string& path, bool absolute = false);

	private:
		std::string modelDir;
		std::vector<Ref<Mesh>> m_Meshes;
		std::vector<Ref<SkinnedMesh>> m_SkinnedMeshes;

		void ProcessNode(aiNode* node, const aiScene* scene);
		Ref<Mesh> ProcessMesh(aiMesh* node, const aiScene* scene);

		std::vector<Vertex> ProcessVertices(aiMesh* mesh);
		std::vector<uint32_t> ProcessIndices(aiMesh* mesh);
		Ref<Material> ProcessMaterials(const aiScene* scene, aiMesh* mesh);
		Ref<Texture> ProcessTextures(const aiScene* scene, const std::string& path);

		// Skinned
		void ProcessSkinnedNode(aiNode* node, const aiScene* scene);
		Ref<SkinnedMesh> ProcessSkinnedMesh(aiMesh* node, const aiScene* scene);
		std::vector<SkinnedVertex> ProcessSkinnedVertices(aiMesh* mesh);
	};
}
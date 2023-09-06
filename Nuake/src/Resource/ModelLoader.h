#pragma once
#include "src/Core/Core.h"

#include "src/Rendering/Mesh/SkinnedMesh.h"
#include "src/Rendering/Mesh/Mesh.h"
#include "src/Rendering/Textures/Material.h"
#include "src/Rendering/Textures/Texture.h"

#include "src/Rendering/Vertex.h"
#include "src/Resource/SkeletonNode.h"

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
		void SetVertexBoneData(SkinnedVertex& vertex, int boneId, float weight);
		void ProcessAnimationNode(SkeletonNode& dest, const aiNode* src);

		static inline Matrix4 ConvertMatrixToGLMFormat(const aiMatrix4x4& from)
		{
			Matrix4 result;
			for (auto i = 0; i < 3; i++) 
			{
				for (auto j = 0; j < 3; j++) 
				{
					result[i][j] = from[i][j];
				}
			}

			// The rest would be zero, other than the 4,4.
			result[0][3] = 0.0f;
			result[1][3] = 0.0f;
			result[2][3] = 0.0f;

			result[3][0] = 0.0f;
			result[3][1] = 0.0f;
			result[3][2] = 0.0f;

			result[3][3] = 1.0f;

			return result;
		}
	};
}
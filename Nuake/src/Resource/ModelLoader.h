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
		std::unordered_map<std::string, uint32_t> m_BoneIDMap;
		std::unordered_map<std::string, Bone> m_BoneMap;

		void ProcessNode(aiNode* node, const aiScene* scene);
		Ref<Mesh> ProcessMesh(aiMesh* mesh, aiNode* node, const aiScene* scene);

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
		void ProcessSkeleton(SkeletonNode& des, const aiNode* src);

		static inline Matrix4 ConvertMatrixToGLMFormat(const aiMatrix4x4& from)
		{
			Matrix4 to;

			to[0][0] = from.a1; to[0][1] = from.b1;  to[0][2] = from.c1; to[0][3] = from.d1;
			to[1][0] = from.a2; to[1][1] = from.b2;  to[1][2] = from.c2; to[1][3] = from.d2;
			to[2][0] = from.a3; to[2][1] = from.b3;  to[2][2] = from.c3; to[2][3] = from.d3;
			to[3][0] = from.a4; to[3][1] = from.b4;  to[3][2] = from.c4; to[3][3] = from.d4;

			return to;
		}
	};
}
#include "IAssetBaker.h"

#include "Nuake/Rendering/Vertex.h"

// Assimp
#include "assimp/Importer.hpp"
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <vector>

namespace Nuake
{
	struct BakerMaterialData
	{
		std::string albedo;
		std::string normal;
		std::string ao;
		std::string roughness;
		std::string metallic;
	};
	
	struct MeshData
	{
		std::vector<Vertex> vertices;
		std::vector<uint32_t> indices;
		BakerMaterialData material;
	};
	
	class GLTFBaker : public IAssetBaker
	{
	public:
		GLTFBaker() : IAssetBaker({".glb", ".gltf", "fbx"}) {}
		
		Ref<File> Bake(const Ref<File>& file) override;
		
	private:
		std::map<std::string, BakerMaterialData> Materials;
		
		void ProcessNode(aiNode* node, const aiScene* scene, std::vector<MeshData>& meshes);
		MeshData ProcessMesh(aiNode* node, aiMesh* meshNode, const aiScene* scene);
		std::vector<Vertex> ProcessVertices(aiMesh* mesh);
		std::vector<uint32_t> ProcessIndices(aiMesh* mesh);
		BakerMaterialData ProcessMaterials(const aiScene* scene, aiMesh* mesh);
		std::string ProcessTextures(const aiScene* scene, const std::string& path);
		
	private:
		static inline Matrix4 ConvertAssimpToGLM(const aiMatrix4x4& from)
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
#include "Nuake/Resource/Bakers/IAssetBaker.h"

#include "Nuake/Rendering/Vertex.h"


#include <vector>

struct aiNode;
struct aiScene;
struct aiMesh;

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
	private:
		std::string currentPath;

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
	};
}
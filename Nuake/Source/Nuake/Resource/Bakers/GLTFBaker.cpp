#include "GLTFBaker.h"

#include "Nuake/Core/Logger.h"
#include "Nuake/Core/String.h"
#include "Nuake/FileSystem/Directory.h"
#include "Nuake/Rendering/Textures/Material.h"
#include "Nuake/Rendering/Mesh/Mesh.h"
#include "Nuake/Resource/Model.h"
#include "Nuake/Resource/Serializer/BinarySerializer.h"

#include <assimp/matrix4x4.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

using namespace Nuake;

// Converts a GLTF file to a .nkmesh binary file
// TODO(antopilo): Add support for multiple file extensions
Ref<File> GLTFBaker::Bake(const Ref<File>& file)
{
	// 1. Read file
	// 2. Convert
	// 3. return output file
	// NOTE: This function should not interact with the rest
	// of the engine and be thread-safe in order to be ran from jobs.
	
	// Load into assimp
	Assimp::Importer importer;
	importer.SetPropertyFloat("PP_GSN_MAX_SMOOTHING_ANGLE", 90);
	
	const std::string absolutePath = file->GetAbsolutePath();
	auto importFlags = 
		aiProcess_Triangulate | 
		aiProcess_GenSmoothNormals |
		aiProcess_FixInfacingNormals | 
		aiProcess_CalcTangentSpace;
	const aiScene* scene = importer.ReadFile(absolutePath, importFlags);
	
	if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) // Assimp failed
	{
		std::string assimpErrorMsg = std::string(importer.GetErrorString());
		std::string logMsg = "" + assimpErrorMsg;
		Logger::Log(logMsg, "GLTF Baker", CRITICAL);
		return nullptr;
	}
	
	// Processing of the data
	std::vector<MeshData> meshesData;
	ProcessNode(scene->mRootNode, scene, meshesData);
	
	importer.FreeScene();
	
	// Write to disk
	std::vector<Mesh> meshes;
	
	std::map<std::string, Ref<Material>> materialCache;
	Ref<Model> model = CreateRef<Model>();
	for(auto& meshData : meshesData)
	{
		Ref<Mesh> mesh = CreateRef<Mesh>();
		mesh->SetData(meshData.vertices, meshData.indices);
		
		const BakerMaterialData materialData = meshData.material;

		std::string materialPath;
		Ref<Material> material;
		if (!materialData.albedo.empty())
		{
			materialPath = FileSystem::GetParentPath(file->GetRelativePath()) +  FileSystem::GetFileNameFromPath(materialData.albedo) + ".material";
		}

		if (materialCache.find(materialPath) != materialCache.end())
		{
			material = materialCache[materialPath];
		}
		else
		{
			material = CreateRef<Material>();
			if (!materialData.albedo.empty())
			{
				material->SetAlbedo(absolutePath + "/../" + materialData.albedo);
			}

			if (!materialData.normal.empty())
			{
				material->SetNormal(absolutePath + "/../" + materialData.normal);
			}

			if (!materialData.ao.empty())
			{
				material->SetAO(absolutePath + "/../" + materialData.ao);
			}

			if (!materialData.metallic.empty())
			{
				material->SetMetalness(absolutePath + "/../" + materialData.metallic);
			}

			if (!materialData.roughness.empty())
			{
				material->SetRoughness(absolutePath + "/../" + materialData.roughness);
			}


			ResourceManager::RegisterResource(material);
			ResourceManager::Manifest.RegisterResource(material->ID, materialPath);

			std::string materialJson = material->Serialize().dump(4);
			FileSystem::BeginWriteFile(materialPath);
			FileSystem::WriteLine(materialJson);
			FileSystem::EndWriteFile();

			materialCache[materialPath] = material;
		}

		mesh->MaterialResource = RID(material->ID);
		model->AddMesh(std::move(mesh));
	}
	
	// Bake 
	BinarySerializer serializer;
	
	const std::string outputPath = file->GetAbsolutePath() + ".nkmesh";
	serializer.SerializeModel(outputPath, model);
	
	//Ref<Model> model = CreateRef<Model>();
	return file;
}

void GLTFBaker::ProcessNode(aiNode* node, const aiScene* scene, std::vector<MeshData>& meshes)
{
	for(uint32_t i = 0; i < node->mNumMeshes; i++)
	{
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		meshes.push_back(ProcessMesh(node, mesh, scene));
	}
	
	for(uint32_t i = 0; i < node->mNumChildren; i++)
	{
		ProcessNode(node->mChildren[i], scene, meshes);
	}
}

Matrix4 ConvertAssimpToGLM(const aiMatrix4x4& from)
{
	Matrix4 to;

	to[0][0] = from.a1; to[0][1] = from.b1;  to[0][2] = from.c1; to[0][3] = from.d1;
	to[1][0] = from.a2; to[1][1] = from.b2;  to[1][2] = from.c2; to[1][3] = from.d2;
	to[2][0] = from.a3; to[2][1] = from.b3;  to[2][2] = from.c3; to[2][3] = from.d3;
	to[3][0] = from.a4; to[3][1] = from.b4;  to[3][2] = from.c4; to[3][3] = from.d4;

	return to;
}

MeshData GLTFBaker::ProcessMesh(aiNode* node, aiMesh* meshNode, const aiScene* scene)
{
	std::vector<Vertex> vertices = ProcessVertices(meshNode);
	for(auto& vertex : vertices)
	{
		const Matrix4& modelTransform = ConvertAssimpToGLM(node->mTransformation);
		vertex.position = modelTransform * Vector4(vertex.position, 1.0f);
	}
	
	std::vector<uint32_t> indices = ProcessIndices(meshNode);
	MeshData meshData
	{
		std::move(vertices),
		std::move(indices),
		ProcessMaterials(scene, meshNode)
	};
	
	return std::move(meshData);
}

std::vector<Vertex> GLTFBaker::ProcessVertices(aiMesh* mesh)
{
	auto vertices = std::vector<Vertex>();
	
	const uint32_t numVertices = mesh->mNumVertices;
	vertices.reserve(numVertices);
	
	for(uint32_t i = 0; i < numVertices; i++)
	{
		Vertex vertex {};
		vertex.uv_x = 0.0f;
		vertex.uv_y = 0.0f;
		
		// Position
		auto assimpVec3 = mesh->mVertices[i];
		Vector3 position
		{ 
			assimpVec3.x,
			assimpVec3.y,
			assimpVec3.z
		};
		vertex.position = std::move(position);
		
		// Normal
		assimpVec3 = mesh->mNormals[i];
		Vector3 normal
		{
			assimpVec3.x,
			assimpVec3.y,
			assimpVec3.z
		};
		vertex.normal = std::move(normal);
		
		if(mesh->mTangents)
		{
			vertex.tangent = 
			Vector4 {
				mesh->mTangents[i].x,
				mesh->mTangents[i].y,
				mesh->mTangents[i].z,
				0.0f
			};
		}
		
		if(mesh->mBitangents)
		{
			vertex.bitangent = 
				Vector4 {
				mesh->mBitangents[i].x,
				mesh->mBitangents[i].y,
				mesh->mBitangents[i].z,
				0.0f
			};
		}
		
		if(mesh->mTextureCoords[0])
		{
			vertex.uv_x = mesh->mTextureCoords[0][i].x;
			vertex.uv_y = mesh->mTextureCoords[0][i].y;
		}
		
		vertices.push_back(vertex);
	}
	
	return std::move(vertices);
}

std::vector<uint32_t> GLTFBaker::ProcessIndices(aiMesh* mesh)
{
	auto indices = std::vector<uint32_t>();
	for (uint32_t i = 0; i < mesh->mNumFaces; i++)
	{
		aiFace face = mesh->mFaces[i];
		for (uint32_t j = 0; j < face.mNumIndices; j++)
		{
			indices.push_back(face.mIndices[j]);
		}
	}
	
	return std::move(indices);
}

BakerMaterialData GLTFBaker::ProcessMaterials(const aiScene* scene, aiMesh* mesh)
{
	BakerMaterialData materialData { };
	
	if(mesh->mMaterialIndex < 0)
	{
		return materialData;
	}
	
	aiMaterial* materialNode = scene->mMaterials[mesh->mMaterialIndex];
	
	aiString aiMaterialName;	
	materialNode->Get(AI_MATKEY_NAME,  aiMaterialName);
	
	const std::string materialName = std::string(aiMaterialName.C_Str());
	if(Materials.find(materialName) != Materials.end())
	{
		return materialData;
	}
	
	aiString str;
	if(materialNode->GetTextureCount(aiTextureType_DIFFUSE) > 0)
	{
		materialNode->GetTexture(aiTextureType_DIFFUSE, 0, &str);
		materialData.albedo = std::string(str.C_Str());
	}
	
	if(materialNode->GetTextureCount(aiTextureType_NORMALS) > 0)
	{
		materialNode->GetTexture(aiTextureType_NORMALS, 0, &str);
		materialData.normal = std::string(str.C_Str());
	}
	
	if(materialNode->GetTextureCount(aiTextureType_AMBIENT_OCCLUSION) > 0)
	{
		materialNode->GetTexture(aiTextureType_AMBIENT_OCCLUSION, 0, &str);
		materialData.ao = std::string(str.C_Str());
	}
	
	if(materialNode->GetTextureCount(aiTextureType_DIFFUSE_ROUGHNESS) > 0)
	{
		materialNode->GetTexture(aiTextureType_DIFFUSE_ROUGHNESS, 0, &str);
		materialData.roughness = std::string(str.C_Str());
	}
	
	return std::move(materialData);
}

std::string GLTFBaker::ProcessTextures(const aiScene* scene, const std::string& path)
{
	if(String::BeginsWith(path, "*"))
	{
		// TODO(antopilo): Figure out how to handle embedded textures.
		
		Logger::Log("Embedded textures not supported", "GLTFBaker", WARNING);
		return "";
	}
	
	return std::move(path);
}



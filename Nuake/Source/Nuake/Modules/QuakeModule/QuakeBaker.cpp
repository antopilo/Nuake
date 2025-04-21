#include "QuakeBaker.h"

#include "QuakeBSPReader.h"
#include <Nuake/Rendering/Vertex.h>
#include <Nuake/Resource/Model.h>
#include <Nuake/FileSystem/Directory.h>
#include <Nuake/Rendering/Textures/Material.h>
#include <Nuake/Core/String.h>

#include <Nuake/Rendering/Vulkan/VulkanImage/VulkanImage.h>
#include <stb_image/stb_image_write.h>

using namespace Nuake;

static const char* ExpandFileName(const char* lump_name, bool fullbright)
{
	int max_len = strlen(lump_name) + 32;

	char* result = (char*)calloc(max_len + 1, 1); (max_len);

	// convert any special first character
	if (lump_name[0] == '*')
	{
		strcpy(result, "star_");
	}
	else if (lump_name[0] == '+')
	{
		strcpy(result, "plus_");
	}
	else if (lump_name[0] == '-')
	{
		strcpy(result, "minu_");
	}
	else if (lump_name[0] == '/')
	{
		strcpy(result, "divd_");
	}
	else if (lump_name[0] == '{')
	{
		strcpy(result, "bra_");
	}
	else
	{
		result[0] = lump_name[0];
		result[1] = 0;
	}

	strcat(result, lump_name + 1);

	// sanitize filename (remove problematic characters)
	bool warned = false;

	for (char* p = result; *p; p++)
	{
		if (*p == ' ')
			*p = '_';

		if (*p != '_' && *p != '-' && !isalnum(*p))
		{
			if (!warned)
			{
				printf("WARNING: removing weird characters from name (\\%03o)\n",
					(unsigned char)*p);
				warned = true;
			}

			*p = '_';
		}
	}

	//if (fullbright)
	//	strcat(result, "_fbr");

	strcat(result, ".png");

	return result;
}

Ref<File> QuakeBaker::Bake(const Ref<File>& file)
{
	if (!BSPParser::LoadFile(file->GetAbsolutePath()))
	{
		return file;
	}

	BSPParser::Parse();

	std::map<std::string, std::string> textureFileMap;

	std::map<std::string, std::shared_ptr<VulkanImage>> textures;
	for (auto& tex : BSPParser::MipTextures)
	{
		const std::string& textureName = tex.name;
		auto data = BSPParser::GetTextureData(textureName);
		
		const std::string& fileName = std::string(ExpandFileName(textureName.c_str(), false));
		auto finalPath = file->GetAbsolutePath() + "/../" + "/" + fileName;
		stbi_write_png(finalPath.c_str(), tex.width, tex.height, 4, data.data(), tex.width * 4);
		textureFileMap[textureName] = fileName;
		//auto texture = CreateRef<VulkanImage>(reinterpret_cast<void*>(data.data()), ImageFormat::RGBA8, Vector2{tex.width, tex.height});
		//textures[textureName] = texture;
	}

	Ref<Model> model = CreateRef<Model>();

    // Copy vertices
    uint32_t verticesNum = BSPParser::Vertices.size();
    std::vector<Vertex> vertices;
    for (const auto& v : BSPParser::Vertices)
    {
        vertices.push_back({
            Vector3(-v.X, v.Z, v.Y),
            0.0f,
            Vector3(v.X, v.Z, v.Y),
            0.0f,
        });
    }

    std::map<std::string, std::vector<Vertex>> verticesByTexture;
    std::map<std::string, std::vector<unsigned int>> indicesByTexture;

	auto& m = BSPParser::Models[0];
	//for (const auto& m : BSPParser::Models)
	{
		for (int i = m.face_id; i < m.face_id + m.face_num; i++)
		{
			const auto& face = BSPParser::Faces[i];
			long firstEdge = face.ledge_id;
			long lastEdge = firstEdge + face.ledge_num;

			auto textureInfo = BSPParser::TextureInfos[face.texinfo_id];
			const std::string& textureName = BSPParser::MipTextures[textureInfo.texture_id].name;

			std::vector<int32_t> faceIndices;

			for (long e = firstEdge; e < lastEdge; e++)
			{
				int32_t edgeId = BSPParser::LEdges[e];
				if (edgeId >= 0)
				{
					auto edge = BSPParser::Edges[edgeId];
					faceIndices.push_back(edge.vertex0);
				}
				else
				{
					auto edge = BSPParser::Edges[-edgeId];
					faceIndices.push_back(edge.vertex1);
				}
			}

			auto calculateUV = [&](Vector3 pos, BSPParser::surface_t surface) -> Vector2
			{
				Vector3 vectorS = { -surface.vectorS.x, surface.vectorS.z, surface.vectorS.y };
				Vector3 vectorT = { -surface.vectorT.x, surface.vectorT.z, surface.vectorT.y };
				float s = glm::dot(pos, vectorS) + surface.distS;
				float t = glm::dot(pos, vectorT) + surface.distT;
				return { (s / BSPParser::MipTextures[surface.texture_id].width), 1.0 - (t / BSPParser::MipTextures[surface.texture_id].height) };
			};

			if (faceIndices.size() >= 3)
			{
				auto& verts = verticesByTexture[textureName];
				auto& inds = indicesByTexture[textureName];

				for (size_t j = 1; j < faceIndices.size() - 1; ++j)
				{
					size_t baseIndex = verts.size();

					Vector3 p0 = vertices[faceIndices[0]].position * Vector3(1.0 / 64.0);
					Vector3 p1 = vertices[faceIndices[j]].position * Vector3(1.0 / 64.0);
					Vector3 p2 = vertices[faceIndices[j + 1]].position * Vector3(1.0 / 64.0);

					Vector2 uv0 = Vector2{ 1.0 , 1.0 } - calculateUV(vertices[faceIndices[0]].position, textureInfo);
					Vector2 uv1 = Vector2{ 1.0 , 1.0 } - calculateUV(vertices[faceIndices[j]].position, textureInfo);
					Vector2 uv2 = Vector2{ 1.0 , 1.0 } - calculateUV(vertices[faceIndices[j + 1]].position, textureInfo);

					Vector3 edge1 = p1 - p0;
					Vector3 edge2 = p2 - p0;
					Vector3 normal = -glm::normalize(glm::cross(edge1, edge2));

					verts.push_back({ .position = p0, .uv_x = uv0.x, .normal = normal, .uv_y = uv0.y });
					verts.push_back({ .position = p1, .uv_x = uv1.x, .normal = normal, .uv_y = uv1.y});
					verts.push_back({ .position = p2, .uv_x = uv2.x, .normal = normal, .uv_y = uv2.y});

					inds.push_back(baseIndex + 2);
					inds.push_back(baseIndex + 1);
					inds.push_back(baseIndex + 0);
				}
			}
		}
	}

	std::vector<Mesh*> triangleMeshes;
	for (auto& [textureName, verts] : verticesByTexture)
	{
		if (String::BeginsWith(textureName, "sky"))
		{
			continue;
		}

		auto& inds = indicesByTexture[textureName];

		Ref<Material> material = CreateRef<Material>();
		auto materialPath = FileSystem::GetParentPath(file->GetRelativePath()) + FileSystem::GetFileNameFromPath(textureFileMap[textureName]) + ".material";
		auto texturePath = FileSystem::GetParentPath(file->GetRelativePath()) + FileSystem::GetFileNameFromPath(textureFileMap[textureName]) + ".png";
		material->SetAlbedo(texturePath);

		ResourceManager::RegisterResource(material);
		ResourceManager::Manifest.RegisterResource(material->ID, materialPath);

		std::string materialJson = material->Serialize().dump(4);
		FileSystem::BeginWriteFile(materialPath);
		FileSystem::WriteLine(materialJson);
		FileSystem::EndWriteFile();

		Ref<Mesh> mesh = CreateRef<Mesh>();
		mesh->SetData(verts, inds);
		mesh->SetMaterial(material);
		model->AddMesh(std::move(mesh));
	}

	BinarySerializer serializer;

	const std::string outputPath = file->GetAbsolutePath() + ".nkmesh";
	serializer.SerializeModel(outputPath, model);

	BSPParser::UnloadFile();

	return file;
}

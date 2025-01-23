#include "BinarySerializer.h"

#include "src/Resource/Resource.h"
#include "src/Resource/Model.h"
#include "src/Rendering/Textures/Material.h"

#include <iostream>
#include <fstream>

using namespace Nuake;

struct BinaryMesh
{
	uint64_t RID;
	uint64_t MaterialRID;
	uint32_t VertexCount;
	std::vector<Vertex> Vertices;
	uint32_t IndexCount;
	std::vector<uint32_t> Indices;
};

struct BinaryModel
{
	uint64_t RID;
	uint32_t MeshCount;
	std::vector<BinaryMesh> Meshes;
};

class BinaryReader
{
private:
	std::ifstream File;

public:
	BinaryReader(const std::string& path)
	{
		File = std::ifstream(path, std::ios::binary);
	}

	~BinaryReader()
	{
		File.close();
	}

public:
	bool IsOpen() const { return File.is_open(); }

	template<typename T>
	void Read(T* data, uint32_t count = 1)
	{
		File.read(reinterpret_cast<char*>(data), count * sizeof(T));
	}
};

class BinaryWriter
{
private:
	std::ofstream File;

public:
	BinaryWriter(const std::string& path)
	{
		File = std::ofstream(path, std::ios::binary);
	}

	~BinaryWriter()
	{
		File.close();
	}

public:
	bool IsOpen() const { return File.is_open(); }
	template<typename T>
	void Write(T* data, uint32_t count = 1)
	{
		File.write(reinterpret_cast<char*>(data), count * sizeof(T));
	}
};

UUID BinarySerializer::DeserializeUUID(const std::string& path)
{
	BinaryReader reader = BinaryReader(FileSystem::RelativeToAbsolute(path));
	if (!reader.IsOpen())
	{
		return UUID(0);
	}

	uint64_t rid;
	reader.Read<uint64_t>(&rid);
	return rid;
}

Ref<Material> BinarySerializer::DeserializeMaterial(const std::string& path)
{
	BinaryReader reader = BinaryReader(path);
	if (!reader.IsOpen())
	{
		return nullptr;
	}

	return nullptr;
}

bool BinarySerializer::SerializeMaterial(const std::string& path, Ref<Material> material)
{
	BinaryWriter writer = BinaryWriter(path);
	return true;
}

Ref<Model> BinarySerializer::DeserializeModel(const std::string& path)
{
	BinaryReader reader = BinaryReader(FileSystem::RelativeToAbsolute(path));
	if (!reader.IsOpen())
	{
		return nullptr;
	}

	BinaryModel binaryModel{ };
	reader.Read<uint64_t>(&binaryModel.RID);
	reader.Read<uint32_t>(&binaryModel.MeshCount);

	for (int i = 0; i < binaryModel.MeshCount; i++)
	{
		BinaryMesh binaryMesh{};
		reader.Read<uint64_t>(&binaryMesh.MaterialRID);
		reader.Read<uint32_t>(&binaryMesh.VertexCount);
		binaryMesh.Vertices.resize(binaryMesh.VertexCount);
		reader.Read<Vertex>(binaryMesh.Vertices.data(), binaryMesh.VertexCount);

		reader.Read<uint32_t>(&binaryMesh.IndexCount);
		binaryMesh.Indices.resize(binaryMesh.IndexCount);
		reader.Read<uint32_t>(binaryMesh.Indices.data(), binaryMesh.IndexCount);

		binaryModel.Meshes.push_back(std::move(binaryMesh));
	}

	Ref<Model> model = CreateRef<Model>();
	model->ID = binaryModel.RID;
	for (auto& binaryMesh : binaryModel.Meshes)
	{
		Ref<Mesh> mesh = CreateRef<Mesh>();
		mesh->AddSurface(binaryMesh.Vertices, binaryMesh.Indices);
		mesh->MaterialResource = RID(binaryMesh.MaterialRID);
		model->AddMesh(mesh);
	}

	return model;
}

bool BinarySerializer::SerializeModel(const std::string& path, Ref<Model> model)
{
	const uint32_t meshCount = static_cast<uint32_t>(model->GetMeshes().size());
	BinaryModel binaryModel
	{
		.RID = model->ID,
		.MeshCount = meshCount
	};

	binaryModel.Meshes.reserve(meshCount);

	for (auto& mesh : model->GetMeshes())
	{
		BinaryMesh binaryMesh
		{
			.MaterialRID = static_cast<uint64_t>(mesh->MaterialResource.ID),
			.VertexCount = static_cast<uint32_t>(mesh->GetVertices().size()),
			.Vertices = mesh->GetVertices(),
			.IndexCount = static_cast<uint32_t>(mesh->GetIndices().size()),
			.Indices = mesh->GetIndices()
		};

		binaryModel.Meshes.push_back(binaryMesh);
	}

	// Write data to file
	{
		BinaryWriter writer(path);
		if (!writer.IsOpen())
		{
			return false;
		}

		writer.Write<uint64_t>(&binaryModel.RID);
		writer.Write<uint32_t>(&binaryModel.MeshCount);
		for (int i = 0; i < binaryModel.MeshCount; i++)
		{
			BinaryMesh& binaryMesh = binaryModel.Meshes[i];
			writer.Write<uint64_t>(&binaryMesh.MaterialRID);
			writer.Write<uint32_t>(&binaryMesh.VertexCount);
			writer.Write<Vertex>(binaryMesh.Vertices.data(), binaryMesh.VertexCount);
			writer.Write<uint32_t>(&binaryMesh.IndexCount);
			writer.Write<uint32_t>(binaryMesh.Indices.data(), binaryMesh.IndexCount);
		}
	}

	return true;
}

Ref<Mesh> BinarySerializer::DeserializeMesh(const std::string& path)
{
	std::ifstream file(path, std::ios::binary);
	if (!file.is_open())
	{
		return nullptr;
	}

	BinaryMesh binaryMesh;
	file.read((char*)&binaryMesh.VertexCount, sizeof(uint32_t));
	binaryMesh.Vertices.resize(binaryMesh.VertexCount);
	file.read((char*)binaryMesh.Vertices.data(), binaryMesh.VertexCount * sizeof(Vertex));
	file.read((char*)&binaryMesh.IndexCount, sizeof(uint32_t));
	binaryMesh.Indices.resize(binaryMesh.IndexCount);
	file.read((char*)binaryMesh.Indices.data(), binaryMesh.IndexCount * sizeof(uint32_t));
	file.close();

	Ref<Mesh> mesh = CreateRef<Mesh>();
	mesh->AddSurface(binaryMesh.Vertices, binaryMesh.Indices);
	return mesh;
}

bool BinarySerializer::SerializeMesh(const std::string& path, Ref<Mesh> mesh)
{
	BinaryMesh binaryMesh;
	binaryMesh.VertexCount = mesh->GetVertices().size();
	binaryMesh.Vertices = mesh->GetVertices();
	binaryMesh.IndexCount = mesh->GetIndices().size();
	binaryMesh.Indices = mesh->GetIndices();

	std::ofstream file(path, std::ios::binary);
	if (!file.is_open())
	{
		return false;
	}

	file.write((char*)&binaryMesh.VertexCount, sizeof(uint32_t));
	file.write((char*)binaryMesh.Vertices.data(), binaryMesh.VertexCount * sizeof(Vertex));
	file.write((char*)&binaryMesh.IndexCount, sizeof(uint32_t));
	file.write((char*)binaryMesh.Indices.data(), binaryMesh.IndexCount * sizeof(uint32_t));
	file.close();

	return true;
}
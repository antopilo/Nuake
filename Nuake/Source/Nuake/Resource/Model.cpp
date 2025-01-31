#include "Nuake/Resource/Model.h"
#include "Nuake/FileSystem/FileSystem.h"
#include "Nuake/Core/Logger.h"
#include "Nuake/Rendering/Mesh/Mesh.h"
#include "Nuake/Resource/ModelLoader.h"
#include "Serializer/BinarySerializer.h"

namespace Nuake
{
	Model::Model(const std::string path) : m_Meshes(std::vector<Ref<Mesh>>())
	{
		this->Path = path;
	}

	Model::Model() : m_Meshes(std::vector<Ref<Mesh>>())
	{}

	Model::~Model() {}

	void Model::AddMesh(Ref<Mesh> mesh)
	{
		m_Meshes.push_back(mesh);
	}

	std::vector<Ref<Mesh>>& Model::GetMeshes()
	{
		return m_Meshes;
	}

	json Model::SerializeData()
	{
		BEGIN_SERIALIZE();
		j["UUID"] = static_cast<uint64_t>(ID);
		for (uint32_t i = 0; i < std::size(m_Meshes); i++)
		{
			j["Meshes"][i] = m_Meshes[i]->Serialize();
		}
		END_SERIALIZE();
	}

	json Model::Serialize()
	{
		BEGIN_SERIALIZE();

		if (this->Path != "")
		{
			j["Path"] = this->Path;
		}
		else
		{
			j["UUID"] = static_cast<uint64_t>(ID);

			BinarySerializer serializer;
			for (uint32_t i = 0; i < std::size(m_Meshes); i++)
			{
				serializer.SerializeMesh(FileSystem::RelativeToAbsolute("mesh" + std::to_string(i) + ".nkmesh"), m_Meshes[i]);
				j["Meshes"][i] = m_Meshes[i]->Serialize();
			}
		}
		END_SERIALIZE();
	}

	bool Model::Deserialize(const json& j)
	{
		if (j.contains("Path"))
		{
			

			ModelLoader loader;
			auto otherModel = loader.LoadModel(j["Path"], false);
			m_Meshes = otherModel->GetMeshes();

			this->Path = j["Path"];
		}
		else
		{
			if (j.contains("UUID"))
			{
				ID = UUID(j["UUID"]);
			}

			if (j.contains("Meshes"))
			{
				for (auto& m : j["Meshes"])
				{
					Ref<Mesh> mesh = CreateRef<Mesh>();
					mesh->Deserialize(m);
					m_Meshes.push_back(mesh);
				}
			}
		}

		return true;
	}
}
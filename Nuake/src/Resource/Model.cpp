#include "src/Resource/Model.h"
#include "src/FileSystem/FileSystem.h"
#include "src/Core/Logger.h"
#include "src/Rendering/Mesh/Mesh.h"
#include "src/Resource/ModelLoader.h"

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
			for (uint32_t i = 0; i < std::size(m_Meshes); i++)
			{
				j["Meshes"][i] = m_Meshes[i]->Serialize();
			}
		}
		END_SERIALIZE();
	}

	bool Model::Deserialize(const json& j)
	{
		if (j.contains("Path"))
		{
			this->IsEmbedded = true;

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
					//Ref<Mesh> mesh = CreateRef<VkMesh>();
					//mesh->Deserialize(m);
					//m_Meshes.push_back(mesh);
				}
			}
		}

		return true;
	}
}
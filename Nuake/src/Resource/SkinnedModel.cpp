#include "src/Resource/ModelLoader.h"

#include "src/Resource/SkinnedModel.h"
#include "src/Core/FileSystem.h"
#include "src/Core/Logger.h"


namespace Nuake
{
	SkinnedModel::SkinnedModel(const std::string path) : m_Meshes(std::vector<Ref<SkinnedMesh>>())
	{
		this->Path = path;
	}

	SkinnedModel::SkinnedModel() : m_Meshes(std::vector<Ref<SkinnedMesh>>())
	{}

	SkinnedModel::~SkinnedModel() {}

	void SkinnedModel::AddMesh(Ref<SkinnedMesh> mesh)
	{
		m_Meshes.push_back(mesh);
	}

	std::vector<Ref<SkinnedMesh>>& SkinnedModel::GetMeshes()
	{
		return m_Meshes;
	}

	json SkinnedModel::Serialize()
	{
		BEGIN_SERIALIZE();

		if (this->Path != "")
		{
			j["Path"] = this->Path;
		}
		else
		{
			for (uint32_t i = 0; i < std::size(m_Meshes); i++)
			{
				j["Meshes"][i] = m_Meshes[i]->Serialize();
			}
		}
		END_SERIALIZE();
	}

	bool SkinnedModel::Deserialize(const json& j)
	{
		if (j.contains("Path"))
		{
			this->IsEmbedded = true;

			ModelLoader loader;
			auto otherModel = loader.LoadSkinnedModel(j["Path"], false);
			m_Meshes = otherModel->GetMeshes();

			this->Path = j["Path"];
		}
		else
		{
			for (auto& m : j["Meshes"])
			{
				auto mesh = CreateRef<SkinnedMesh>();
				mesh->Deserialize(m);

				m_Meshes.push_back(mesh);
			}
		}

		return true;
	}
}
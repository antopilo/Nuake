#include "src/Resource/Model.h"
#include "src/Core/FileSystem.h"
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

	json Model::Serialize()
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

	bool Model::Deserialize(const std::string& str)
	{
		BEGIN_DESERIALIZE();
		if (j.contains("Path"))
		{
			this->IsEmbedded = true;

			ModelLoader loader;
			auto otherModel = loader.LoadModel(j["Path"]);
			m_Meshes = otherModel->GetMeshes();

			this->Path = j["Path"];
		}
		else
		{
			for (auto& m : j["Meshes"])
			{
				Ref<Mesh> mesh = CreateRef<Mesh>();
				mesh->Deserialize(m.dump());

				m_Meshes.push_back(mesh);
			}
		}

		return true;
	}
}
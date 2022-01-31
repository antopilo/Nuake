#include "MaterialManager.h"

#include <string>
#include <fstream>
#include <streambuf>

#include "json/json.hpp"

#include "../Rendering/Textures/Material.h"
#include "src/Core/FileSystem.h"
#include "src/Core/Logger.h"

namespace Nuake
{
	using json = nlohmann::json;

	Ref<MaterialManager> MaterialManager::s_Instance;

	MaterialManager::MaterialManager()
	{
		m_Materials = std::map<std::string, Ref<Material>>();

		GetMaterial(DEFAULT_MATERIAL);
	}

	Ref<Material> MaterialManager::GetMaterial(std::string name)
	{
		if (!IsMaterialLoaded(name))
		{
			Ref<Material> newMaterial = CreateRef<Material>(name);
			RegisterMaterial(newMaterial);
			return newMaterial;
		}

		return m_Materials[name];
	}

	void MaterialManager::RegisterMaterial(Ref<Material> material)
	{
		m_Materials[material->GetName()] = material;
	}

	// Gets a material from path and load file if not already loaded.
	//
	// NOTE: The path passed as param is relative to Res/Materials and 
	//       shouldn't contain the '.material' extension.
	Ref<Material> MaterialManager::LoadMaterial(const std::string materialPath) 
	{
		if (IsMaterialLoaded(materialPath))
			return (m_Materials)[materialPath];

		std::string finalPath = "Res/Materials/" + materialPath + ".material";
		std::ifstream i(finalPath);
		json j;
		i >> j;

		std::string matName;
		if (!j.contains("name")) 
		{
			std::string msg = "Error: Cannot load material file: " + materialPath +
				" - Material file must have a name. \n";
			printf(msg.c_str());
			return nullptr;
		}
		else 
		{
			matName = j["name"];
		}

		std::string albedoPath;
		std::string normalPath;
		std::string aoPath;
		std::string metalPath;
		std::string roughnessPath;
		std::string displacementPath;

		Ref<Material> newMaterial = CreateRef<Material>(j["albedo"]);
		//if (j.contains("albedo"))
		//	newMaterial = new Material(albedoPath);
		//else
		//	newMaterial = new Material(glm::vec3(1, 1, 1));
		newMaterial->SetName(matName);

		if (j.contains("normal"))
			newMaterial->SetNormal(j["normal"]);
		if (j.contains("ao"))
			newMaterial->SetAO(j["ao"]);
		if (j.contains("metal"))
			newMaterial->SetMetalness(j["metal"]);
		if (j.contains("roughness"))
			newMaterial->SetRoughness(j["roughness"]);
		if (j.contains("displacement"))
			newMaterial->SetDisplacement(j["displacement"]);

		return newMaterial;
	}



	std::map<std::string, Ref<Material>> MaterialManager::GetAllMaterials()
	{
		return m_Materials;
	}

	bool MaterialManager::IsMaterialLoaded(const std::string materialPath) 
	{
		return m_Materials.find(materialPath) != m_Materials.end();
	}
}



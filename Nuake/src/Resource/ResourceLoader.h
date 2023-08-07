#pragma once
#include "src/Core/Core.h"
#include "src/Core/Logger.h"
#include "src/Resource/Resource.h"
#include "src/Resource/ResourceManager.h"
#include "src/Rendering/Textures/Material.h"

namespace Nuake
{
	class ResourceLoader
	{
	private:
		const std::string FILE_NOT_FOUND = "[Resource Loader] File doesn't exists. \n ";
		const std::string WRONG_EXTENSION = "[Resource Loader] Resource type mismatch file extension. \n expected: ";
		const std::string MATERIAL_EXT = ".material";

	public:
		ResourceLoader() = default;
		~ResourceLoader() = default;

		static Ref<Material> LoadMaterial(const std::string& path)
		{
			const std::string FILE_NOT_FOUND = "[Resource Loader] File doesn't exists. \n ";
			const std::string WRONG_EXTENSION = "[Resource Loader] Resource type mismatch file extension. \n expected: ";
			const std::string MATERIAL_EXT = ".material";
			if (!FileSystem::FileExists(path))
			{
				Logger::Log(FILE_NOT_FOUND + path, "resource", LOG_TYPE::WARNING);
				return nullptr;
			}

			if (!String::EndsWith(path, MATERIAL_EXT))
			{
				std::string message = WRONG_EXTENSION + MATERIAL_EXT + " actual: " + path;
				Logger::Log(message, "resource", LOG_TYPE::WARNING);
			}

			std::string content = FileSystem::ReadFile(path);
			json j = json::parse(content);

			UUID uuid = ReadUUID(j);

			// Check if resource is already loaded.
			if (ResourceManager::IsResourceLoaded(uuid))
			{
				return ResourceManager::GetResource<Material>(uuid);
			}

			Ref<Material> material = CreateRef<Material>();
			material->ID = uuid;
			material->Path = path;
			material->Deserialize(j);
			ResourceManager::RegisterResource(material);

			return material;
		}
	private:
		static UUID ReadUUID(json j)
		{
			if (j.contains("UUID"))
			{
				return UUID(j["UUID"]);
			}

			return UUID();
		}
	};
}
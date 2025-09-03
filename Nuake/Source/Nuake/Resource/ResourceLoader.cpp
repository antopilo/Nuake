#include "ResourceLoader.h"
#include "Nuake/Core/Logger.h"
#include "Nuake/Core/String.h"
#include "Nuake/FileSystem/File.h"
#include "Nuake/Resource/Resource.h"
#include "Nuake/Resource/ResourceManager.h"
#include "Nuake/Rendering/Textures/Material.h"
#include "Nuake/Resource/Model.h"
#include "Nuake/Resource/UI.h"
#include "Nuake/Resource/SkyResource.h"
#include "Nuake/Scene/Lighting/Environment.h"

using namespace Nuake;

Ref<Material> ResourceLoader::LoadMaterial(const std::string& path)
{
	const std::string FILE_NOT_FOUND = "[Resource Loader] File doesn't exists. \n ";
	const std::string WRONG_EXTENSION = "[Resource Loader] Resource type mismatch file extension. \n expected: ";
	const std::string MATERIAL_EXT = ".material";
	if (path.empty())
	{
		Logger::Log(FILE_NOT_FOUND + path, "resource", LOG_TYPE::WARNING);
		return nullptr;
	}

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
	return ResourceManager::GetResource<Material>(uuid);
}

Ref<Model> ResourceLoader::LoadModel(const std::string& path)
{
	const std::string FILE_NOT_FOUND = "[Resource Loader] File doesn't exists. \n ";
	const std::string WRONG_EXTENSION = "[Resource Loader] Resource type mismatch file extension. \n expected: ";
	const std::string MESH_EXT = ".mesh";
	if (path.empty())
	{
		Logger::Log(FILE_NOT_FOUND + path, "resource", LOG_TYPE::WARNING);
		return nullptr;
	}

	if (!FileSystem::FileExists(path))
	{
		Logger::Log(FILE_NOT_FOUND + path, "resource", LOG_TYPE::WARNING);
		return nullptr;
	}

	if (!String::EndsWith(path, MESH_EXT))
	{
		std::string message = WRONG_EXTENSION + MESH_EXT + " actual: " + path;
		Logger::Log(message, "resource", LOG_TYPE::WARNING);
	}

	std::string content = FileSystem::ReadFile(path);
	json j = json::parse(content);

	UUID uuid = ReadUUID(j);

	// Check if resource is already loaded.
	if (ResourceManager::IsResourceLoaded(uuid))
	{
		return ResourceManager::GetResource<Model>(uuid);
	}

	Ref<Model> model = CreateRef<Model>();
	model->ID = uuid;
	model->Path = path;
	model->Deserialize(j);
	ResourceManager::RegisterResource(model);

	return model;
}

Ref<SkyResource> ResourceLoader::LoadSky(const std::string& path)
{
	const std::string FILE_NOT_FOUND = "[Resource Loader] File doesn't exists. \n ";
	const std::string WRONG_EXTENSION = "[Resource Loader] Resource type mismatch file extension. \n expected: ";
	const std::string SKY_EXT = ".sky";
	if (path.empty())
	{
		Logger::Log(FILE_NOT_FOUND + path, "resource", LOG_TYPE::WARNING);
		return nullptr;
	}

	if (!FileSystem::FileExists(path))
	{
		Logger::Log(FILE_NOT_FOUND + path, "resource", LOG_TYPE::WARNING);
		return nullptr;
	}

	if (!String::EndsWith(path, SKY_EXT))
	{
		std::string message = WRONG_EXTENSION + SKY_EXT + " actual: " + path;
		Logger::Log(message, "resource", LOG_TYPE::WARNING);
	}

	std::string content = FileSystem::ReadFile(path);
	json j = json::parse(content);

	UUID uuid = ReadUUID(j);

	// Check if resource is already loaded.
	if (ResourceManager::IsResourceLoaded(uuid))
	{
		return ResourceManager::GetResource<SkyResource>(uuid);
	}

	Ref<SkyResource> sky = CreateRef<SkyResource>();
	sky->ID = uuid;
	sky->Path = path;
	sky->Deserialize(j);
	ResourceManager::RegisterResource(sky);

	return sky;
}

Ref<Environment> ResourceLoader::LoadEnvironment(const std::string& path)
{
	const std::string FILE_NOT_FOUND = "[Resource Loader] File doesn't exists. \n ";
	const std::string WRONG_EXTENSION = "[Resource Loader] Resource type mismatch file extension. \n expected: ";
	const std::string ENV_EXT = ".env";

	if (path.empty())
	{
		Logger::Log(FILE_NOT_FOUND + path, "resource", LOG_TYPE::WARNING);
		return nullptr;
	}

	if (!FileSystem::FileExists(path))
	{
		Logger::Log(FILE_NOT_FOUND + path, "resource", LOG_TYPE::WARNING);
		return nullptr;
	}

	if (!String::EndsWith(path, ENV_EXT))
	{
		std::string message = WRONG_EXTENSION + ENV_EXT + " actual: " + path;
		Logger::Log(message, "resource", LOG_TYPE::WARNING);
	}

	std::string content = FileSystem::ReadFile(path);
	json j = json::parse(content);

	UUID uuid = ReadUUID(j);

	// Check if resource is already loaded.
	if (ResourceManager::IsResourceLoaded(uuid))
	{
		return ResourceManager::GetResource<Environment>(uuid);
	}

	Ref<Environment> env = CreateRef<Environment>();
	env->ID = uuid;
	env->Path = path;
	env->Deserialize(j);
	ResourceManager::RegisterResource(env);

	return env;
}


Ref<UIResource> ResourceLoader::LoadUI(const std::string& path)
{
	auto uiResource = CreateRef<UIResource>(path);
	return uiResource;
}

Ref<UIResource> ResourceLoader::LoadUI(const Ref<File>& file)
{
	if (file == nullptr || !file->Exist())
	{
		return nullptr;
	}
	
	return LoadUI(file->GetRelativePath());
}

UUID ResourceLoader::ReadUUID(json j)
{
	if (j.contains("UUID"))
	{
		return UUID(j["UUID"]);
	}

	return UUID();
}

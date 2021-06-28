#include "Project.h"
#include "../Core/FileSystem.h"
#include <json/json.hpp>
#include <fstream>
#include <streambuf>
#include "../Core/Logger.h"
Project::Project(const std::string Name, const std::string Description, const std::string& FullPath, const std::string& defaultScenePath )
{
	this->Name = Name;
	this->Description = Description;
	this->FullPath = FullPath;

	if (defaultScenePath != "")
	{
		Ref<Scene> scene = CreateRef<Scene>();
		scene->Deserialize(defaultScenePath);
	}

	this->EntityDefinitionsFile = CreateRef<FGDFile>();

	SaveAs(FullPath);
}

Project::Project()
{
	this->Name = "";
	this->Description = "";
	this->FullPath = "";

	this->EntityDefinitionsFile = CreateRef<FGDFile>();
}

void Project::Save()
{
	SaveAs(this->FullPath);
}

void Project::SaveAs(const std::string FullPath)
{

	json j = Serialize();
	// Dump.
	std::string serialized_string = j.dump();

	// write to file.
	std::ofstream projectFile;
	projectFile.open(FullPath);
	projectFile << serialized_string;
	projectFile.close();
}

Ref<Project> Project::New(const std::string Name, const std::string Description, const std::string FullPath)
{
	if (Name == "")
		return nullptr;

	return CreateRef<Project>(Name, Description, FullPath);
}

Ref<Project> Project::New()
{
	return CreateRef<Project>();
}

Ref<Project> Project::Load(std::string path)
{
	std::ifstream i(path);
	json j;
	i >> j;

	// validation
	std::string projectName = "";
	if(!j.contains("ProjectName"))
		return nullptr;

	projectName = j["ProjectName"];

	std::string description = "";
	if (j.contains("Description"))
		description = j["Description"];

	return CreateRef<Project>(projectName, description, path);
}

json Project::Serialize()
{
	BEGIN_SERIALIZE();
	SERIALIZE_VAL(Name);
	SERIALIZE_VAL(Description);
	SERIALIZE_VAL_LBL("DefaultScene", DefaultScene->Path);
	END_SERIALIZE();
}

bool Project::Deserialize(const std::string& str)
{
	json j = json::parse(str);
	if (!j.contains("Name"))
		return false;
	if (!j.contains("Description"))
		return false;

	this->DefaultScene = Scene::New();

	if (j.contains("DefaultScene"))
	{
		std::string scenePath = j["DefaultScene"];
		if (scenePath != "")
		{
			std::string sceneContent = FileSystem::ReadFile(scenePath, false);
			if (!this->DefaultScene->Deserialize(sceneContent))
			{
				Logger::Log("Error loading scene: " + scenePath, CRITICAL);
			}
			else
			{
				this->DefaultScene->Path = scenePath;
				Logger::Log("Successfully loaded scene: " + scenePath);
			}
		}
		
	}

	return true; // Success
}

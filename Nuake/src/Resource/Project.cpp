#include "Project.h"
#include "../Core/FileSystem.h"
#include <json/json.hpp>
#include <fstream>
#include <streambuf>
#include "../Core/Logger.h"

namespace Nuake
{
	Project::Project(const std::string& Name, const std::string& Description, const std::string& FullPath, const std::string& defaultScenePath)
	{
		this->Name = Name;
		this->Description = Description;
		this->FullPath = FullPath;
		this->TrenchbroomPath = "";

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
		this->TrenchbroomPath = "";

		this->EntityDefinitionsFile = CreateRef<FGDFile>();
	}

	void Project::Save()
	{
		SaveAs(this->FullPath);
	}

	void Project::SaveAs(const std::string& FullPath)
	{
		json j = Serialize();
		std::string serialized_string = j.dump(4);

		// TODO: Use file interface here...
		// Write to file.
		std::ofstream projectFile;
		projectFile.open(FullPath);
		projectFile << serialized_string;
		projectFile.close();
	}

	bool Project::FileExist()
	{
		return std::filesystem::exists(this->FullPath.c_str());
	}

	Ref<Project> Project::New(const std::string& Name, const std::string& Description, const std::string& FullPath)
	{
		return CreateRef<Project>(Name, Description, FullPath);
	}

	Ref<Project> Project::New()
	{
		return CreateRef<Project>();
	}

	Ref<Project> Project::Load(std::string& path)
	{
		std::ifstream i(path);
		json j;
		i >> j;

		// validation
		std::string projectName;
		if (!j.contains("ProjectName"))
			return nullptr;

		projectName = j["ProjectName"];

		std::string description;
		if (j.contains("Description"))
			description = j["Description"];

		return CreateRef<Project>(projectName, description, path);
	}

	json Project::Serialize()
	{
		BEGIN_SERIALIZE();
		SERIALIZE_VAL(Name);
		SERIALIZE_VAL(Description);

		if(DefaultScene)
			SERIALIZE_VAL_LBL("DefaultScene", DefaultScene->Path);

		if(EntityDefinitionsFile)
			SERIALIZE_VAL_LBL("EntityDefinition", EntityDefinitionsFile->Path);
		SERIALIZE_VAL(TrenchbroomPath);
		END_SERIALIZE();
	}

	bool Project::Deserialize(const json& j)
	{
		if (!j.contains("Name") || !j.contains("Description"))
			return false;

		Name = j["Name"];
		Description = j["Description"];

		if (j.contains("EntityDefinition") && j["EntityDefinition"] != "")
		{
			std::string path = j["EntityDefinition"];
			EntityDefinitionsFile = CreateRef<FGDFile>(path);
			std::string content = FileSystem::ReadFile(path, false);
			EntityDefinitionsFile->Deserialize(nlohmann::json::parse(content));
		}

		if (j.contains("TrenchbroomPath"))
		{
			this->TrenchbroomPath = j["TrenchbroomPath"];
		}

		DefaultScene = Scene::New();

		// Load default scene, a project can have no default scene set.
		if (!j.contains("DefaultScene") )
			return true;

		std::string scenePath = j["DefaultScene"];
		if (scenePath == "") // Not set correctly.
			return true;

		if (!FileSystem::FileExists(scenePath))
			return true;

		std::string sceneContent = FileSystem::ReadFile(scenePath, false);
		if (!DefaultScene->Deserialize(nlohmann::json::parse(sceneContent)))
		{
			Logger::Log("Error loading scene: " + scenePath, "project", CRITICAL);
		}

		DefaultScene->Path = scenePath;
		Logger::Log("Loaded scene: " + scenePath);

		return true; // Success
	}
}


#include "Project.h"
#include "src/Core/FileSystem.h"
#include "Engine.h"
#include "src/Core/Logger.h"
#include "src/Audio/AudioManager.h"
#include "src/Scripting/ScriptingEngineNet.h"

#include <json/json.hpp>

#include <fstream>
#include <streambuf>


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


		Ref<Project> project = CreateRef<Project>(projectName, description, path);

		project->Settings = ProjectSettings();

		if (j.contains("Settings"))
		{
			project->Settings.Deserialize(j["Settings"]);

			Engine::SetPhysicsStep(project->Settings.PhysicsStep);

			AudioManager::Get().SetGlobalVolume(project->Settings.GlobalVolume);
		}

		return project;
	}

	void Project::ExportEntitiesToTrenchbroom()
	{
		Ref<FGDFile> file = EntityDefinitionsFile;

		file->BrushEntities.clear();
		for (auto& [name, type] : ScriptingEngineNet::Get().GetBrushEntities())
		{
			FGDBrushEntity brushEntity = FGDBrushEntity(name);
			brushEntity.Script = name;
			brushEntity.Description = type.Description;
			brushEntity.IsTrigger = type.isTrigger;
			for (auto& t : type.exposedVars)
			{
				ClassProperty classProp;
				classProp.name = t.Name;
				classProp.type = ClassPropertyType::String;

				if (t.Type == ExposedVarTypes::String && t.Value.has_value())
				{
					classProp.value = std::any_cast<std::string>(t.Value);
				}
				else if (t.Type == ExposedVarTypes::Float && t.Value.has_value())
				{
					classProp.value = std::any_cast<float>(t.Value);
					classProp.type = ClassPropertyType::Float;
				}
				else if (t.Type == ExposedVarTypes::Int)
				{
					classProp.value = std::to_string(std::any_cast<int>(t.Value));
				}
				else
				{
					classProp.value = "";
				}

				brushEntity.Properties.push_back(classProp);
			}

			file->BrushEntities.push_back(brushEntity);
		}

		file->PointEntities.clear();
		std::vector<std::string> bases;
		for (auto& [name, type] : ScriptingEngineNet::Get().GetPointEntities()) 
		{
			FGDPointEntity pointEntity = FGDPointEntity(name);
			pointEntity.Script = name;
			pointEntity.Description = type.Description;
			for (auto& t : type.exposedVars)
			{
				ClassProperty classProp;
				classProp.name = t.Name;
				classProp.type = ClassPropertyType::String;
				if (t.Type == ExposedVarTypes::String)
				{
					if (t.Value.has_value())
					{
						classProp.value = std::any_cast<std::string>(t.Value);
					}

					classProp.type = ClassPropertyType::String;
				}
				else if (t.Type == ExposedVarTypes::Int)
				{
					if (t.Value.has_value())
					{
						classProp.value = std::to_string(std::any_cast<int>(t.Value));
					}

					classProp.type = ClassPropertyType::Integer;
				}
				else if (t.Type == ExposedVarTypes::Float)
				{
					if (t.Value.has_value())
					{
						classProp.value = std::to_string(std::any_cast<float>(t.Value));
					}

					classProp.type = ClassPropertyType::Float;
				}
				else if (t.Type == ExposedVarTypes::Bool)
				{
					if (t.Value.has_value())
					{
						classProp.value = std::to_string(std::any_cast<bool>(t.Value));
					}

					classProp.type = ClassPropertyType::Float;
				}
				else
				{
					classProp.value = "";
				}

				pointEntity.Properties.push_back(classProp);
			}

			file->PointEntities.push_back(pointEntity);
		}

		for (auto& b : bases)
		{
			FGDBaseEntity baseEntity;
			baseEntity.Name = b;
			file->BaseEntities.push_back(baseEntity);
		}

		file->Export();
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

		// Project Settings
		j["Settings"] = Settings.Serialize();

		END_SERIALIZE();
	}

	bool Project::Deserialize(const json& j)
	{
		Logger::Log("Starting deserializing project", "window", VERBOSE);
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

		if (j.contains("Settings"))
		{
			Settings = ProjectSettings();
			Settings.Deserialize(j["Settings"]);
			Engine::GetCurrentWindow()->SetVSync(Settings.VSync);

			Engine::SetPhysicsStep(Settings.PhysicsStep);

			AudioManager::Get().SetGlobalVolume(Settings.GlobalVolume);
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

		Logger::Log("Starting deserializing scene", "window", VERBOSE);
		std::string sceneContent = FileSystem::ReadFile(scenePath, false);
		if (!DefaultScene->Deserialize(nlohmann::json::parse(sceneContent)))
		{
			Logger::Log("Error loading scene: " + scenePath, "project", CRITICAL);
		}

		DefaultScene->Path = scenePath;
		Logger::Log("Loaded scene: " + scenePath);

		return true; // Success
	}

	json ProjectSettings::Serialize()
	{
		BEGIN_SERIALIZE();

		SERIALIZE_VAL(VSync);
		SERIALIZE_VAL(ShowGizmos);
		SERIALIZE_VAL(ShowAxis);
		SERIALIZE_VAL(ResolutionScale);
		SERIALIZE_VAL(GizmoSize);
		SERIALIZE_VAL(OutlineRadius);
		SERIALIZE_VEC4(PrimaryColor);
		SERIALIZE_VAL(SmoothCamera);
		SERIALIZE_VAL(SmoothCameraSpeed);
		SERIALIZE_VAL(PhysicsStep);
		SERIALIZE_VAL(MaxPhysicsSubStep);
		SERIALIZE_VAL(MaxPhysicsBodies);
		SERIALIZE_VAL(MaxPhysicsBodyPair);
		SERIALIZE_VAL(MaxPhysicsContactConstraints);
		SERIALIZE_VAL(GlobalVolume);
		SERIALIZE_VAL(MaxActiveVoiceCount);
		END_SERIALIZE();
	}

	bool ProjectSettings::Deserialize(const json& j)
	{
		DESERIALIZE_VAL(VSync);
		DESERIALIZE_VAL(ShowGizmos);
		DESERIALIZE_VAL(ShowAxis);
		DESERIALIZE_VAL(ResolutionScale);
		DESERIALIZE_VAL(OutlineRadius);
		DESERIALIZE_VAL(GizmoSize);
		DESERIALIZE_VAL(SmoothCamera);
		DESERIALIZE_VAL(SmoothCameraSpeed);
		DESERIALIZE_VAL(PhysicsStep);
		DESERIALIZE_VAL(MaxPhysicsSubStep);
		DESERIALIZE_VAL(MaxPhysicsBodies);
		DESERIALIZE_VAL(MaxPhysicsBodyPair);
		DESERIALIZE_VAL(MaxPhysicsContactConstraints);

		DESERIALIZE_VAL(GlobalVolume);
		DESERIALIZE_VAL(MaxActiveVoiceCount);

		if (j.contains("PrimaryColor"))
		{
			DESERIALIZE_VEC4(j["PrimaryColor"], PrimaryColor);
		}

		return true;
	}
}


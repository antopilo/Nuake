#include "FGDFile.h"
#include <src/Resource/FGD/FDGSerializer.h>
#include "src/FileSystem/FileSystem.h"
#include "Engine.h"

namespace Nuake {
	FGDFile::FGDFile(const std::string path)
	{
		this->Path = path;

		this->BaseEntities = std::vector<FGDBaseEntity>();
		this->BrushEntities = std::vector<FGDBrushEntity>();
		this->PointEntities = std::vector<FGDPointEntity>();
	}


	FGDFile::FGDFile()
	{
		this->Path = "";
		this->BaseEntities = std::vector<FGDBaseEntity>();
		this->BrushEntities = std::vector<FGDBrushEntity>();
		this->PointEntities = std::vector<FGDPointEntity>();
	}


	bool FGDFile::Save()
	{
		// Write to file.
		FileSystem::BeginWriteFile(Path);
		FileSystem::WriteLine(Serialize().dump(4));
		FileSystem::EndWriteFile();

		return true;
	}

	bool FGDFile::SaveAs()
	{
		return false;
	}


	void FGDFile::AddClass(FGDClass fgdClass)
	{
		//this->Classes.push_back(fgdClass);
	}


	void FGDFile::RemoveClass(const std::string& name)
	{
		//int idx = 0;
		//for (auto& c : Classes)
		//{
			//if (c.Name == name)
				//Classes.erase(Classes.begin() + idx);

			//idx++;
		//}
	}


	// Exports the FGD data into an fgd file that Trenchbroom can read.
	// Should be exported in the Trenchbroom directory directly. /Games/project_name
	bool FGDFile::Export()
	{
		Ref<Project> project = Engine::GetProject();

		if (project->Name.empty())
		{
			Logger::Log("Failed to create game configuration file: project name cannot be empty", "trenchbroom", CRITICAL);
			return false;
		}

		// Create the games/PROJECT_NAME/ directories if they don't exist.
		const std::string gamesDirectoryPath = project->TrenchbroomPath + "/../games/";
		if (!FileSystem::DirectoryExists(gamesDirectoryPath, true))
		{
			FileSystem::MakeDirectory(gamesDirectoryPath, true);
		}

		const std::string configDirectoryPath = gamesDirectoryPath + project->Name;
		if (!FileSystem::DirectoryExists(configDirectoryPath, true))
		{
			FileSystem::MakeDirectory(configDirectoryPath, true);
			Logger::Log("Created game config directory: " + configDirectoryPath, "trenchbrrom");
		}

		// Write the file
		const std::string filePath = configDirectoryPath + "/" + project->Name + ".fgd";
		FGDSerializer::BeginFGDFile(filePath);

		for (auto& b : BaseEntities)
		{

		}

		for (auto& p : PointEntities)
		{
			FGDSerializer::SerializePoint(p);
		}

		for (auto& b : BrushEntities)
		{
			FGDSerializer::SerializeBrush(b);
		}

		//for(auto& c : Classes)
		//	FGDSerializer::SerializeClass(c);

		FGDSerializer::EndFGDFile();

		return true;
	}
}

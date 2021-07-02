#include "FGDFile.h"
#include <src/Resource/FGD/FDGSerializer.h>
#include <src/Core/FileSystem.h>
#include "Engine.h"

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

	// TODO: Remove this, debugging stuff.
	FGDPointEntity newEnt;
	newEnt.Name = "Light";
	newEnt.Description = "A Nuake PBR light";
	
	this->PointEntities.push_back(newEnt);
}


bool FGDFile::Save()
{
	// Write to file.
	FileSystem::BeginWriteFile(FileSystem::Root + Path);
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
	std::string path = project->TrenchbroomPath + "games/" + project->Name + "/" + project->Name + ".fgd";


	FGDSerializer::BeginFGDFile(path);

	for (auto& b : BaseEntities)
	{

	}

	for (auto& p : PointEntities)
	{

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
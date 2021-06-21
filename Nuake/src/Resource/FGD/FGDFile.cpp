#include "FGDFile.h"
#include <src/Resource/FGD/FDGSerializer.h>

FGDFile::FGDFile(const std::string path)
{
	this->path = path;

	this->BaseEntities = std::vector<FGDBaseEntity>();
	this->BrushEntities = std::vector<FGDBrushEntity>();
	this->PointEntities = std::vector<FGDPointEntity>();
}

FGDFile::FGDFile()
{
	this->path = "";
	this->BaseEntities = std::vector<FGDBaseEntity>();
	this->BrushEntities = std::vector<FGDBrushEntity>();
	this->PointEntities = std::vector<FGDPointEntity>();

	FGDPointEntity newEnt;
	newEnt.Name = "Light";
	newEnt.Description = "A Nuake PBR light";
	
	this->PointEntities.push_back(newEnt);
}

bool FGDFile::Save()
{
	FGDSerializer::BeginFGDFile(this->path);

	for (auto& b : BaseEntities)
	{

	}

	for (auto& p : PointEntities)
	{

	}

	for (auto& b : BrushEntities)
	{

	}
	//for(auto& c : Classes)
	//	FGDSerializer::SerializeClass(c);

	FGDSerializer::EndFGDFile();
	return true;
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

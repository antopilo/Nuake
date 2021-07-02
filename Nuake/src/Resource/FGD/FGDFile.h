#pragma once
#include "FGDClass.h"
#include "../Serializable.h"
#include <string>
#include <vector>

enum class EntityType
{
	Brush, Point, Base, None
};

class FGDFile : ISerializable{
public:
	std::string Path;

	std::vector<FGDPointEntity> PointEntities;
	std::vector<FGDBrushEntity> BrushEntities;
	std::vector<FGDBaseEntity> BaseEntities;

	FGDFile(const std::string path);
	FGDFile();

	bool Export();
	bool Save();
	bool SaveAs();

	void AddClass(FGDClass fgdClass);	
	void RemoveClass(const std::string& name);

	json Serialize() override
	{
		BEGIN_SERIALIZE();
		int i = 0;
		for (auto& c : this->BrushEntities)
		{
			j["BrushEntities"][i] = c.Serialize();
			i++;
		}

		i = 0;
		for (auto& c : this->PointEntities)
		{
			j["PointEntities"][i] = c.Serialize();
			i++;
		}
		END_SERIALIZE();
	}

	bool Deserialize(const std::string& str)
	{
		BEGIN_DESERIALIZE();

		if (j.contains("BrushEntities"))
		{
			for (auto& brush : j["BrushEntities"])
			{
				FGDBrushEntity brushEntity = FGDBrushEntity();
				brushEntity.Deserialize(brush.dump());
				BrushEntities.push_back(brushEntity);
			}
		}

		if (j.contains("PointEntities"))
		{

		}

		return true;
	}

	FGDBrushEntity& GetBrushEntity(const std::string& name)
	{
		for (auto& b : BrushEntities)
		{
			if (b.Name == name)
				return b;
		}
	}

	EntityType GetTypeOfEntity(const std::string& className)
	{
		for (auto& b : BrushEntities)
		{
			if (b.Name == className)
				return EntityType::Brush;
		}

		for (auto& p : PointEntities)
		{
			if (p.Name == className)
				return EntityType::Point;
		}

		return EntityType::None;
	}
};
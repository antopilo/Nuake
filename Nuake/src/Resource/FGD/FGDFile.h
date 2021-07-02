#pragma once
#include "FGDClass.h"
#include "../Serializable.h"
#include <string>
#include <vector>

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
};
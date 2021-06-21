#pragma once
#include "FGDClass.h"
#include "../Serializable.h"
#include <string>
#include <vector>

class FGDFile : ISerializable{
public:
	std::string path;

	std::vector<FGDPointEntity> PointEntities;
	std::vector<FGDBrushEntity> BrushEntities;
	std::vector<FGDBaseEntity> BaseEntities;

	FGDFile(const std::string path);

	FGDFile();

	bool Save();

	void AddClass(FGDClass fgdClass);
	void RemoveClass(const std::string& name);

	json Serialize() override
	{
		BEGIN_SERIALIZE();

		END_SERIALIZE();
	}

	bool Deserialize(const std::string& str)
	{
		return true;
	}
};
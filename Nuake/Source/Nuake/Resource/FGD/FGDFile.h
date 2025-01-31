#pragma once
#include "FGDClass.h"
#include "../Serializable.h"
#include <string>
#include <vector>

namespace Nuake {
	enum class EntityType
	{
		Brush, Point, Base, None
	};

	class FGDFile : ISerializable 
	{
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
			for (int i = 0; i < BrushEntities.size(); i++)
				j["BrushEntities"][i] = BrushEntities[i].Serialize();

			for (int i = 0; i < PointEntities.size(); i++)
				j["PointEntities"][i] = PointEntities[i].Serialize();
			END_SERIALIZE();
		}

		bool Deserialize(const json& j)
		{
			if (j.contains("BrushEntities"))
			{
				for (auto& brush : j["BrushEntities"])
				{
					FGDBrushEntity brushEntity = FGDBrushEntity();
					brushEntity.Deserialize(brush);
					BrushEntities.push_back(brushEntity);
				}
			}

			if (j.contains("PointEntities"))
			{
				for (auto& point : j["PointEntities"])
				{
					FGDPointEntity pointEntity = FGDPointEntity();
					pointEntity.Deserialize(point);
					PointEntities.push_back(pointEntity);
				}
			}

			return true;
		}

		FGDBrushEntity* GetBrushEntity(const std::string& name)
		{
			for (auto& b : BrushEntities)
			{
				if (String::ToLower(name) == String::ToLower(b.Name))
					return &b;
			}

			assert(false && "Brush entity not found!");

			return nullptr;
		}

		FGDPointEntity* GetPointEntity(const std::string& name)
		{
			for (auto& p : PointEntities)
			{
				if (String::ToLower(name) == String::ToLower(p.Name))
				{
					return &p;
				}
			}

			assert(false && "Point entity not found!");

			return nullptr;
		}

		EntityType GetTypeOfEntity(const std::string& className)
		{
			for (auto& b : BrushEntities)
			{
				if (String::ToLower(b.Name) == String::ToLower(className))
					return EntityType::Brush;
			}

			for (auto& p : PointEntities)
			{
				if (String::ToLower(p.Name) == String::ToLower(className))
					return EntityType::Point;
			}

			return EntityType::None;
		}
	};
}

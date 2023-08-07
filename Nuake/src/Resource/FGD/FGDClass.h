#pragma once
#include "ClassProperty.h"
#include "src/Resource/Serializable.h"
#include "src/Core/String.h"
#include <vector>

namespace Nuake {
	class FGDBaseEntity
	{
	public:
		std::string Name;
		std::vector<ClassProperty> Properties;
	};


	class FGDBrushEntity : ISerializable
	{
	public:
		std::string Name;
		std::string Description;
		std::string Script = "";
		std::string Class = "";

		bool Visible = false;
		bool Solid = false;
		bool IsTrigger = false;

		std::vector<ClassProperty> Properties;

		FGDBrushEntity()
		{
			Name = "";
			Description = "";
		}

		FGDBrushEntity(const std::string& name)
		{
			Name = name;
			Description = "";
		}

		json Serialize() override
		{
			BEGIN_SERIALIZE();
			{
				SERIALIZE_VAL(Name);
				SERIALIZE_VAL(Description);
				SERIALIZE_VAL(Script);
				SERIALIZE_VAL(Class);
				SERIALIZE_VAL(Visible);
				SERIALIZE_VAL(Solid);
				SERIALIZE_VAL(IsTrigger);
			}
			END_SERIALIZE();
		}

		bool Deserialize(const json& j) override
		{
			Name = j["Name"];
			Description = j["Description"];
			Visible = j["Visible"];
			Solid = j["Solid"];
			IsTrigger = j["IsTrigger"];
			Script = j["Script"];
			Class = j["Class"];
			return true;
		}

		FGDBaseEntity BaseClass;
	};


	class FGDPointEntity : ISerializable
	{
	public:
		std::string Name;
		std::string Description;
		std::string Prefab;
		std::vector<ClassProperty> Properties;
		FGDBaseEntity BaseClass;

		FGDPointEntity(const std::string name)
		{
			Name = name;
			Description = "";
		}

		FGDPointEntity()
		{
			Name = "";
			Description = "";
		}

		json Serialize() override
		{
			BEGIN_SERIALIZE();
			SERIALIZE_VAL(Name);
			SERIALIZE_VAL(Description);
			SERIALIZE_VAL(Prefab);
			END_SERIALIZE();
		}

		bool Deserialize(const json& j) override
		{
			Name = j["Name"];
			Description = j["Description"];
			Prefab = j["Prefab"];
			return true;
		}
	};

	class FGDClass 
	{
	public:
		FGDClassType Type;
		std::string Name;
		std::string Description;
		std::vector<ClassProperty> Properties;

		FGDClass(FGDClassType type, const std::string& name, const std::string& desc);

		void AddProperty(ClassProperty prop);
		void RemoveProperty(const std::string name);
	};
}

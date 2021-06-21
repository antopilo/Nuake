#pragma once
#include "ClassProperty.h"
#include <string>
#include <vector>

class FGDBaseEntity
{
public:
	std::string Name;
	std::vector<ClassProperty> Properties;
};

class FGDBrushEntity
{
public:
	std::string Name;
	std::string Description;
	bool Transparent = false;
	bool Collision = false;
	bool IsTrigger = false;
	std::string Script = "";
	std::vector<ClassProperty> Properties;

	FGDBaseEntity BaseClass;
};

class FGDPointEntity
{
public:
	std::string Name;
	std::string Description;
	std::string Prefab;
	std::vector<ClassProperty> Properties;
	FGDBaseEntity BaseClass;
};

class FGDClass {
public:
	FGDClassType Type;
	std::string Name;
	std::string Description;
	std::vector<ClassProperty> Properties;

	FGDClass(FGDClassType type, const std::string& name, const std::string& desc);
	void AddProperty(ClassProperty prop);
	void RemoveProperty(const std::string name);
};
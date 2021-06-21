#include "FGDClass.h"


FGDClass::FGDClass(FGDClassType type, const std::string& name, const std::string& desc)
{
	Type = type;
	Name = name;
	Description = desc;
}

void FGDClass::AddProperty(ClassProperty prop)
{
	Properties.push_back(prop);
}

void FGDClass::RemoveProperty(const std::string name)
{
	int position = 0;
	for (auto& p : Properties)
	{
		if (p.name == name)
			Properties.erase(Properties.begin() + position);

		position++;
	}
}

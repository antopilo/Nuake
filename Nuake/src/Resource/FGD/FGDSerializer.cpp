#include "FDGSerializer.h"
#include <src\Core\FileSystem.h>


bool FGDSerializer::BeginFGDFile(const std::string path)
{
	FileSystem::BeginWriteFile(path);
	return true;
}

bool FGDSerializer::SerializeClass(FGDClass fgdClass)
{
	std::string line = "@";

	if (fgdClass.Type == FGDClassType::Point)
		line += "PointClass = ";
	else if (fgdClass.Type == FGDClassType::Brush)
		line += "SolidClass = ";
	else // error.
		return true;

	line += fgdClass.Name + " : \"" + fgdClass.Description + "\"";

	// Exit early
	if (fgdClass.Properties.size() == 0)
	{
		line += " [] \n";
		FileSystem::WriteLine(line);
		return true;
	}

	// Properties here.
	line += "\n [ \n";
	for (auto& p : fgdClass.Properties)
	{
		// E.g: myProp(integer) : "description"
		line += "    "; // Tabulation
		line += p.name;
		line += "(";
		if (p.type == ClassPropertyType::Integer)
			line += "integer";
		line += ") : ";
		line += "\"" + p.description + "\"";
		line += "\n";

	}

	line += "]";

	FileSystem::WriteLine(line);
	return true;
}

bool FGDSerializer::EndFGDFile()
{
	FileSystem::EndWriteFile();
	return false;
}

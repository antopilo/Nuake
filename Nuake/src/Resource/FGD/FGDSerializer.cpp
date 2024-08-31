#include "FDGSerializer.h"
#include <src/Core/FileSystem.h>

namespace Nuake {
	bool FGDSerializer::BeginFGDFile(const std::string path)
	{
		FileSystem::BeginWriteFile(path, true);
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

	bool FGDSerializer::SerializePoint(FGDPointEntity fgdPoint)
	{
		std::string line = "@";
		line += "PointClass = ";
		line += fgdPoint.Name + " : \"" + fgdPoint.Description + "\"";

		// Exit early
		if (fgdPoint.Properties.size() == 0)
		{
			line += " [] \n";
			FileSystem::WriteLine(line);
			return true;
		}

		// Properties here.
		line += "\n [ \n";
		for (auto& p : fgdPoint.Properties)
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

	bool FGDSerializer::SerializeBrush(FGDBrushEntity fgdClass)
	{
		std::string line = "@";
		line += "SolidClass = ";
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
			if (p.type == ClassPropertyType::String)
				line += "string";
			line += ") : ";
			line += "\"" + p.description + "\"";
			line += " : ";
			if (p.type == ClassPropertyType::String)
			{
				line += " \"" + p.value + "\" ";
			}
			else if (p.type == ClassPropertyType::Integer)
			{
				line += p.value;
			}
			line += "\n";

		}

		line += "]";

		FileSystem::WriteLine(line);
		return false;
	}

	bool FGDSerializer::EndFGDFile()
	{
		FileSystem::EndWriteFile();
		return false;
	}
}


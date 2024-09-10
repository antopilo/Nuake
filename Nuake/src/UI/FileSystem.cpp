#include "FileSystem.h"
#include <fstream>
#include <filesystem>

namespace NuakeUI
{
	namespace FileSystem
	{
		bool FileExists(const std::string& path)
		{
			return std::filesystem::exists(path);
		}

		std::string ReadFile(const std::string& path)
		{
			if (!FileExists(path))
				return "";

			std::string fileContent;
			std::string currentLine;

			std::ifstream file(path);

			while (getline(file, currentLine))
			{
				fileContent += currentLine + "\n";
			}

			file.close();

			return fileContent;
		}
	}
}
#pragma once
#include <string>

namespace NuakeUI
{
	namespace FileSystem
	{
		bool FileExists(const std::string& path);
		std::string ReadFile(const std::string& path);
	};
}

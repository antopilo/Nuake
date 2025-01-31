#pragma once
#include "Nuake/Core/Core.h"

namespace Nuake
{
	class File;

	class Directory
	{
	public:
		std::string Name;
		std::string FullPath;
		Ref<Directory> Parent;
		std::vector<Ref<Directory>> Directories;
		std::vector<Ref<File>> Files;

	public:
		Directory(const std::string& path);

		Directory() = default;
		~Directory() = default;

		std::string GetName() const;
		std::string GetFullPath() const;
	};
}
#pragma once
#include "src/Core/Core.h"

#include "Directory.h"
#include "FileTypes.h"

namespace Nuake
{
	class File
	{
	public:
		File(Ref<Directory> parentDir, const std::string& absolutePath, const std::string& name, const std::string& type);
		~File() = default;

	public:
		std::string GetName() const { return name; }
		std::string GetExtension() const { return type; }
		std::string GetRelativePath() const { return relativePath; }
		std::string GetAbsolutePath() const { return absolutePath; }
		Ref<Directory> GetParent() const { return parent; }
		
		bool GetHasBeenModified() const { return modified; }
		void SetHasBeenModified(bool value) { modified = value; }

		FileType GetFileType() const;
		std::string GetFileTypeAsString() const;

		std::string Read() const;
		bool Exist() const;

	private:
		std::string type;
		std::string name;
		std::string relativePath;
		std::string absolutePath;
		bool modified;

		Ref<Directory> parent;

	};
}

#pragma once
#include "Core.h"
#include "String.h"

#include <string>
#include <filesystem>
#include <iostream>
#include <fstream>

namespace Nuake
{
	class FileDialog
	{
	public:
		static std::string OpenFile(const char* filter);
		static std::string SaveFile(const char* filter);
	};

	class Directory;
	class File;

	class FileSystem
	{
	public:
		static std::string Root;

		static Ref<Directory> RootDirectory;

		static void SetRootDirectory(const std::string path);

		static void Scan();
		static std::string AbsoluteToRelative(const std::string& path);
		static std::string RemoveFileFromPath(const std::string& fullPath);
		static Ref<Directory> GetFileTree();
		static Ref<File> GetFile(const std::string& path);
		static std::string GetFileNameFromPath(const std::string& path);
		static void ScanDirectory(Ref<Directory> directory);
		static void GetDirectories();

		static bool DirectoryExists(const std::string path);
		static bool FileExists(const std::string path, bool absolute = false);

		static std::string ReadFile(const std::string& path, bool absolute = false);

		static std::ofstream fileWriter;
		static bool BeginWriteFile(const std::string path);
		static bool WriteLine(const std::string line);
		static void EndWriteFile();
		static int RemoveFile(const std::string& path);
	};

	class File
	{
	private:
		std::string Type;
		std::string Name;
		std::string RelativePath;
		std::string AbsolutePath;
		Ref<Directory> Parent;
	public:

		std::string GetExtension() { return Type; }
		std::string GetName() { return Name; }
		std::string GetRelativePath() { return RelativePath; }
		std::string GetAbsolutePath() { return AbsolutePath; }
		Ref<Directory> GetParent() { return Parent; }

		std::string Read()
		{
			return FileSystem::ReadFile(AbsolutePath);
		}

		File(Ref<Directory> parentDir, const std::string& absolutePath, const std::string& name, const std::string& type)
		{
			AbsolutePath = absolutePath;
			Parent = parentDir;
			RelativePath = FileSystem::AbsoluteToRelative(absolutePath);
			Name = name;
			Type = type;
		}
	};

	class Directory
	{
	public:
		std::string name;
		std::string fullPath;
		Ref<Directory> Parent;
		std::vector<Ref<Directory>> Directories;
		std::vector<Ref<File>> Files;
	};
}

#pragma once
#include <string>
#include <filesystem>
#include "Core.h"
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

	struct Directory;
	struct File
	{
		std::string Type;
		std::string name;

		std::string fullPath;
		Ref<Directory> Parent;
	};

	struct Directory
	{
		std::string name;
		std::string fullPath;
		Ref<Directory> Parent;
		std::vector<Ref<Directory>> Directories;
		std::vector<Ref<File>> Files;
	};

	class FileSystem
	{
	public:
		static std::string Root;

		static Ref<Directory> RootDirectory;

		static void SetRootDirectory(const std::string path);

		static void Scan();
		static std::string AbsoluteToRelative(const std::string& path);
		static Ref<Directory> GetFileTree();
		static void ScanDirectory(Ref<Directory> directory);
		static void GetDirectories();

		static bool DirectoryExists(const std::string path);
		static bool FileExists(const std::string path);

		static std::string ReadFile(const std::string& path, bool absolute = false);

		static std::ofstream fileWriter;
		static bool BeginWriteFile(const std::string path);
		static bool WriteLine(const std::string line);
		static void EndWriteFile();
	};
}

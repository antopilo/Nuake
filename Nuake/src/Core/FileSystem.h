#pragma once
#include <string>
#include <filesystem>
#include "Core.h"

class __declspec(dllexport) FileDialog
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

	static void Scan();
	static Ref<Directory> GetFileTree();
	static void ScanDirectory(Ref<Directory> directory);
	static void GetDirectories();

	static bool DirectoryExists(const std::string path);
	static bool FileExists(const std::string path);


};
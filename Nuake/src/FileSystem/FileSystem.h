#pragma once
#include "src/Core/Core.h"

namespace filewatch
{
	template<typename T>
	class FileWatch;
}

namespace Nuake
{
	class Directory;
	class File;

	class FileSystem
	{
	public:
		static std::string Root;
		static Ref<Directory> RootDirectory;
		static Ref<filewatch::FileWatch<std::string>> RootFileWatch;

		static void SetRootDirectory(const std::string path);

		static void Scan();
		static std::string AbsoluteToRelative(const std::string& path);
		static std::string RelativeToAbsolute(const std::string& path);
		static std::string GetParentPath(const std::string& fullPath);
		static Ref<Directory> GetFileTree();
		static Ref<File> GetFile(const std::string& path);
		static Ref<Directory> GetDirectory(const std::string& path);
		static std::string GetFileNameFromPath(const std::string& path);
		static void ScanDirectory(Ref<Directory> directory);
		static void GetDirectories();

		static bool MakeDirectory(const std::string& path, bool absolute = false);
		static bool DirectoryExists(const std::string& path, bool absolute = false);
		static bool FileExists(const std::string& path, bool absolute = false);

		static std::string ReadFile(const std::string& path, bool absolute = false);
		static void CopyFileAbsolute(const std::string& src, const std::string& dest);

		static std::ofstream fileWriter;
		static bool BeginWriteFile(const std::string path, bool absolute = false);
		static bool WriteLine(const std::string line);
		static void EndWriteFile();
		static uintmax_t DeleteFileFromPath(const std::string& path);
		static uintmax_t DeleteFolder(const std::string& path);
		static std::string GetConfigFolderPath();
	};
}

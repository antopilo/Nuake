#include "FileSystem.h"

#include "Engine.h"
#include "src/Core/OS.h"
#include "src/Core/String.h"

#include "Directory.h"
#include "File.h"

#include "filewatch/FileWatch.hpp"

#include <filesystem>

using namespace Nuake;

std::string FileSystem::Root = "";

Ref<Directory> FileSystem::RootDirectory;
Ref<filewatch::FileWatch<std::string>> FileSystem::RootFileWatch;

void FileSystem::ForeachFile(OnFileFunc func)
{
	ForeachFile(func, RootDirectory);
}

void FileSystem::ForeachFile(OnFileFunc func, Ref<Directory> dir)
{
	for(auto& file : dir->Files)
	{
		func(file);
	}
	
	for(auto& dir : dir->Directories)
	{
		ForeachFile(func, dir);
	}
}

void FileSystem::ScanDirectory(Ref<Directory> directory)
{
	directory->Files.clear();
	directory->Directories.clear();
	for (const auto& entry : std::filesystem::directory_iterator(directory->FullPath))
	{
		if (entry.is_directory())
		{
			Ref<Directory> newDir = CreateRef<Directory>();
			newDir->FullPath = entry.path().string();
			newDir->Name = entry.path().filename().string();

			newDir->Parent = directory;
			ScanDirectory(newDir);
			directory->Directories.push_back(newDir);
		}
		else if (entry.is_regular_file())
		{
			std::filesystem::path currentPath = entry.path();
			std::string absolutePath = currentPath.string();
			std::string name = currentPath.filename().string();
			std::string extension = currentPath.extension().string();
			Ref<File> newFile = CreateRef<File>(directory, absolutePath, name, extension);
			directory->Files.push_back(newFile);
		}
	}
}

bool FileSystem::DirectoryExists(const std::string& path, bool absolute)
{
	const std::string& finalPath = absolute ? path : Root + path;

	return std::filesystem::exists(finalPath) && std::filesystem::is_directory(finalPath);
}

bool FileSystem::MakeDirectory(const std::string& path, bool absolute)
{
	return std::filesystem::create_directories(absolute ? path : FileSystem::Root + path);
}

bool FileSystem::FileExists(const std::string& path, bool absolute)
{
	std::string fullPath = absolute ? path : FileSystem::Root + path;
	return std::filesystem::exists(fullPath) && std::filesystem::is_regular_file(fullPath);
}

std::vector<Ref<File>> FileSystem::GetAllFiles(const FileType fileType)
{
	std::vector<Ref<File>> foundFiles;

	std::function<void(Ref<Directory>)> scanDir;
	scanDir = [&scanDir, &foundFiles, &fileType](Ref<Directory> dir)
	{
		// All the files matching the filetype
		for (auto& f : dir->Files)
		{
			if (f->GetFileType() == fileType)
			{
				foundFiles.push_back(f);
			}
		}

		// Scan sub folder
		for (auto& d : dir->Directories)
		{
			scanDir(d);
		}
	};

	scanDir(RootDirectory);

	return foundFiles;
}

void FileSystem::SetRootDirectory(const std::string path)
{
	Root = path;
	RootFileWatch = CreateRef<filewatch::FileWatch<std::string>>(
		path, [&](const std::string& path, const filewatch::Event& event)
			{
				std::string normalizedPath = String::ReplaceSlash(path);

				// Detect if its a file and not a folder.
				if (!FileSystem::FileExists(normalizedPath))
				{
					return;
				}

				if (event == filewatch::Event::added)
				{
					const std::string& parent = std::filesystem::path(normalizedPath).parent_path().string();
					auto parentDirectory = FileSystem::GetDirectory(parent);
					auto filePath = std::filesystem::path(normalizedPath);
					std::string name = filePath.filename().string();
					std::string extension = filePath.extension().string();
					Ref<File> newImportedFile = CreateRef<File>(parentDirectory, FileSystem::RelativeToAbsolute(normalizedPath), name, extension);
					parentDirectory->Files.push_back(newImportedFile);
				}

				if(Ref<File> file = GetFile(normalizedPath); file)
				{
					if (file->GetFileType() == FileType::Unknown)
					{
						return;
					}

					if (event == filewatch::Event::modified)
					{
						file->SetHasBeenModified(true);
					}
					
				}
			}
	);
	Scan();
}

void FileSystem::Scan()
{
	RootDirectory = CreateRef<Directory>(Root);
	ScanDirectory(RootDirectory);
}

std::string FileSystem::AbsoluteToRelative(const std::string& path)
{
	const std::filesystem::path rootPath(Root);
	const std::filesystem::path absolutePath(path);
	return std::filesystem::relative(absolutePath, rootPath).generic_string();
}

std::string FileSystem::RelativeToAbsolute(const std::string& path)
{
	return Root + path;
}

std::string FileSystem::GetParentPath(const std::string& fullPath)
{
	std::filesystem::path pathObj(fullPath);
	auto returnvalue = pathObj.parent_path().string();
	return returnvalue + "/";
}

void FileSystem::CopyFileAbsolute(const std::string& src, const std::string& dest)
{
	const std::string& destPath = dest + "/" + std::filesystem::path(src).filename().string();
	std::filesystem::copy_file(src, destPath, std::filesystem::copy_options::overwrite_existing);
}

std::string FileSystem::ReadFile(const std::string& path, bool absolute)
{
	std::string finalPath = path;
	if (!absolute)
		finalPath = Root + path;

	std::ifstream myReadFile(finalPath, std::ios::in | std::ios::binary);
	std::string fileContent = "";
	std::string allFile = "";
		
	char bom[3];
	myReadFile.read(bom, 3);

	// Check for UTF-8 BOM (EF BB BF)
	if (bom[0] == 0xEF && bom[1] == 0xBB && bom[2] == 0xBF) 
	{
		myReadFile.seekg(3);
	}
	else
	{
		myReadFile.seekg(0);
	}

	// Use a while loop together with the getline() function to read the file line by line
	while (getline(myReadFile, fileContent))
	{
		allFile.append(fileContent + "\n");
	}

	// Close the file
	myReadFile.close();
	return allFile;
}

std::ofstream FileSystem::fileWriter;
bool FileSystem::BeginWriteFile(const std::string path, bool absolute)
{
	fileWriter = std::ofstream();
	fileWriter.open(absolute ? path : FileSystem::Root + path);

	return false;
}

bool FileSystem::WriteLine(const std::string line)
{
	fileWriter << line.c_str();

	return true;
}

void FileSystem::EndWriteFile()
{
	fileWriter.close();
}

uintmax_t FileSystem::DeleteFileFromPath(const std::string& path)
{
	return std::remove(path.c_str());
}

uintmax_t FileSystem::DeleteFolder(const std::string& path)
{
	return std::filesystem::remove_all(path.c_str());
}

std::string FileSystem::GetConfigFolderPath()
{
	std::string subFolderPath = OS::GetConfigFolderPath().append("/Nuake/");
	if (!DirectoryExists(subFolderPath, true))
	{
		MakeDirectory(subFolderPath);
	}

	return subFolderPath;
}

Ref<Directory> FileSystem::GetFileTree()
{
	return RootDirectory;
}

Ref<File> FileSystem::GetFile(const std::string& inPath)
{
	std::string path = inPath;
	if (String::BeginsWith(path, "/") || String::BeginsWith(path, "\\"))
	{
		path = inPath.substr(1, inPath.size() - 1);
	}

	// Note, Might be broken on other platforms.
	auto splits = String::Split(path, '/');

	int currentDepth = -1;
	std::string currentDirName = ".";
	Ref<Directory> currentDirComparator = RootDirectory;
	while (currentDirName == currentDirComparator->Name)
	{
		currentDepth++;
		currentDirName = splits[currentDepth];

		// Find next directory
		for (auto& d : currentDirComparator->Directories)
		{
			if (d->Name == currentDirName)
			{
				currentDirComparator = d;
			}
		}

		// Find in files if can't find in directories.
		for (auto& f : currentDirComparator->Files)
		{
			if (f->GetName() == currentDirName || f->GetFullName() == currentDirName)
			{
				return f;
			}
		}
	}

	return nullptr;
}

Ref<Directory> FileSystem::GetDirectory(const std::string& path)
{
	// Note, Might be broken on other platforms.
	auto splits = String::Split(path, '/');

	int currentDepth = -1;
	std::string currentDirName = ".";
	Ref<Directory> currentDirComparator = RootDirectory;
	while (currentDirName == currentDirComparator->Name)
	{
		currentDepth++;

		if (currentDepth >= splits.size())
		{
			return currentDirComparator;
		}

		currentDirName = splits[currentDepth];

		// Find next directory
		for (auto& d : currentDirComparator->Directories)
		{
			if (d->Name == currentDirName)
			{
				currentDirComparator = d;
				continue;
			}
		}
	}

	return currentDirComparator;
}

std::string FileSystem::GetFileNameFromPath(const std::string& path)
{
	const auto& split = String::Split(path, '/');
	return String::Split(split[split.size() - 1], '.')[0];
}

Directory::Directory(const std::string& path)
{
	Files = std::vector<Ref<File>>();
	Directories = std::vector<Ref<Directory>>();
	Name = FileSystem::AbsoluteToRelative(path);
	FullPath = path;
}

std::string Directory::GetName() const
{
	return Name;
}

std::string Directory::GetFullPath() const
{
	return FullPath;
}

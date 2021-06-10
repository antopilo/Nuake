#include "FileSystem.h"
#include <filesystem>

#include "../../Engine.h"
#define GLFW_EXPOSE_NATIVE_WIN32

#include <GLFW/glfw3.h>
#include "GLFW/glfw3native.h"
#include <commdlg.h>
#include <fstream>
#include <iostream>

namespace fs = std::filesystem;

std::string FileDialog::OpenFile(const char* filter)
{

	OPENFILENAMEA ofn;
	CHAR szFile[260] = { 0 };
	ZeroMemory(&ofn, sizeof(OPENFILENAME));
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = glfwGetWin32Window(Engine::GetCurrentWindow()->GetHandle());
	ofn.lpstrFile = szFile;
	ofn.nMaxFile = sizeof(szFile);
	ofn.lpstrFilter = filter;
	ofn.nFilterIndex = 1;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;
	if (GetOpenFileNameA(&ofn) == TRUE)
	{
		return ofn.lpstrFile;
	}
	return std::string();
	
}
std::string FileDialog::SaveFile(const char* filter)
{

	OPENFILENAMEA ofn;
	CHAR szFile[260] = { 0 };
	ZeroMemory(&ofn, sizeof(OPENFILENAME));
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = glfwGetWin32Window(Engine::GetCurrentWindow()->GetHandle());
	ofn.lpstrFile = szFile;
	ofn.nMaxFile = sizeof(szFile);
	ofn.lpstrFilter = filter;
	ofn.nFilterIndex = 1;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;
	if (GetSaveFileNameA(&ofn) == TRUE)
	{
		return ofn.lpstrFile;
	}
	return std::string();

}
std::string FileSystem::Root = "";

Ref<Directory> FileSystem::RootDirectory;

void FileSystem::ScanDirectory(Ref<Directory> directory)
{
	for (const auto& entry : std::filesystem::directory_iterator(directory->fullPath))
	{
		if (entry.is_directory())
		{
			Ref<Directory> newDir = CreateRef<Directory>();
			newDir->fullPath = entry.path().string();
			newDir->name = entry.path().filename().string();

			newDir->Parent = directory;
			ScanDirectory(newDir);

			directory->Directories.push_back(newDir);
		}
		else if (entry.is_regular_file())
		{
			Ref<File> newFile = CreateRef<File>();
			newFile->Type = entry.path().extension().string();
			newFile->name = entry.path().filename().string();
			newFile->Parent = directory;
			newFile->fullPath = entry.path().string();
			directory->Files.push_back(newFile);
		}
	}
}

bool FileSystem::DirectoryExists(const std::string path)
{
	return false;
}

void FileSystem::SetRootDirectory(const std::string path)
{
	Root = path;
	Scan();
}

void FileSystem::Scan()
{
	RootDirectory = CreateRef<Directory>();
	RootDirectory->Files = std::vector<Ref<File>>();
	RootDirectory->Directories = std::vector<Ref<Directory>>();
	RootDirectory->name = Root;
	RootDirectory->fullPath = Root;
	ScanDirectory(RootDirectory);
}

std::string FileSystem::AbsoluteToRelative(const std::string& path)
{
	const fs::path rootPath(Root);
	const fs::path absolutePath(path);
	return fs::relative(absolutePath, rootPath).generic_string();
}

std::string FileSystem::ReadFile(const std::string& path, bool absolute)
{
	std::string finalPath = path;
	if (!absolute)
		finalPath = Root + path;

	std::ifstream MyReadFile(finalPath);
	std::string fileContent = "";
	std::string allFile = "";

	// Use a while loop together with the getline() function to read the file line by line
	while (getline(MyReadFile, fileContent))
	{
		allFile.append(fileContent + "\n");
	}

	// Close the file
	MyReadFile.close();
	return allFile;
}


Ref<Directory> FileSystem::GetFileTree()
{
	return RootDirectory;
}

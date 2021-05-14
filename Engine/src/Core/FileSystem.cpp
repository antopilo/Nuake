#include "FileSystem.h"


#include "../../Engine.h"
#define GLFW_EXPOSE_NATIVE_WIN32

#include <GLFW/glfw3.h>
#include "GLFW/glfw3native.h"
#include <commdlg.h>
#include <fstream>
#include <iostream>

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


std::string FileSystem::Root = "Res\\";
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

void FileSystem::Scan()
{
	RootDirectory = CreateRef<Directory>();
	RootDirectory->Files = std::vector<Ref<File>>();
	RootDirectory->Directories = std::vector<Ref<Directory>>();
	RootDirectory->name = Root;
	RootDirectory->fullPath = Root;
	ScanDirectory(RootDirectory);
}

Ref<Directory> FileSystem::GetFileTree()
{
	return RootDirectory;
}

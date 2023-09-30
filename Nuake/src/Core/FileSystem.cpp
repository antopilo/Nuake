#include "FileSystem.h"

#include "Engine.h"
#include "OS.h"

#include <GLFW/glfw3.h>

#ifdef NK_WIN
#define GLFW_EXPOSE_NATIVE_WIN32

#include "GLFW/glfw3native.h"
#include <commdlg.h>

#endif 

#ifdef NK_LINUX

#include <gtk-3.0/gtk/gtk.h>

#endif

#include <fstream>
#include <iostream>

namespace Nuake
{
	namespace fs = std::filesystem;

	std::string FileDialog::OpenFile(const char* filter)
	{
		std::string filePath;
#ifdef NK_WIN
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
			filePath = std::string(ofn.lpstrFile);
		}
#endif

#ifdef NK_LINUX
		GtkWidget *dialog;
		GtkFileChooserAction action = GTK_FILE_CHOOSER_ACTION_OPEN;
		gint res;

		dialog = gtk_file_chooser_dialog_new("Open File",
			NULL,
			action,
			"_Cancel",
			GTK_RESPONSE_CANCEL,
			"_Open",
			GTK_RESPONSE_ACCEPT,
			NULL);

		gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER(dialog), TRUE);

		if (filter) {
			GtkFileFilter *file_filter = gtk_file_filter_new();
			gtk_file_filter_set_name(file_filter, "Filter Name");
			gtk_file_filter_add_pattern(file_filter, filter);
			gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), file_filter);
		}

		res = gtk_dialog_run(GTK_DIALOG(dialog));

		if (res == GTK_RESPONSE_ACCEPT) {
			char *filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
			filePath = filename;
			g_free(filename);
		}

		gtk_widget_destroy(dialog);
#endif
		return filePath;
	}

	std::string FileDialog::SaveFile(const char* filter)
	{
#ifdef NK_WIN
		OPENFILENAMEA ofn;
		CHAR szFile[260] = { 0 };
		ZeroMemory(&ofn, sizeof(OPENFILENAME));
		ofn.lStructSize = sizeof(OPENFILENAME);
		ofn.hwndOwner = glfwGetWin32Window(Engine::GetCurrentWindow()->GetHandle());
		ofn.lpstrFile = szFile;
		ofn.nMaxFile = sizeof(szFile);
		ofn.lpstrFilter = filter;
		ofn.nFilterIndex = 1;
		ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR | OFN_OVERWRITEPROMPT;
		if (GetSaveFileNameA(&ofn) == TRUE)
		{
			return ofn.lpstrFile;
		}
#endif

#ifdef NK_LINUX
		GtkWidget *dialog;
		GtkFileChooserAction action = GTK_FILE_CHOOSER_ACTION_SAVE;
		gint res;

		gtk_init(NULL, NULL);

		dialog = gtk_file_chooser_dialog_new("Save File",
											 NULL,
											 action,
											 "_Cancel",
											 GTK_RESPONSE_CANCEL,
											 "_Save",
											 GTK_RESPONSE_ACCEPT,
											 NULL);

		GtkFileFilter *file_filter = gtk_file_filter_new();
		gtk_file_filter_set_name(file_filter, filter);
		gtk_file_filter_add_pattern(file_filter, "*.*"); // You can customize this pattern
		gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), file_filter);

		res = gtk_dialog_run(GTK_DIALOG(dialog));
		if (res == GTK_RESPONSE_ACCEPT)
		{
			char *filename;
			GtkFileChooser *chooser = GTK_FILE_CHOOSER(dialog);
			filename = gtk_file_chooser_get_filename(chooser);
			std::string result(filename);
			g_free(filename);
			gtk_widget_destroy(dialog);
			return result;
		}
		else
		{
			gtk_widget_destroy(dialog);
			return std::string();
		}
#endif

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
		return std::filesystem::create_directory(absolute ? path : FileSystem::Root + path);
	}

	bool FileSystem::FileExists(const std::string& path, bool absolute)
	{
		std::string fullPath = absolute ? path : FileSystem::Root + path;
		return std::filesystem::exists(fullPath);
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
		RootDirectory->name = FileSystem::AbsoluteToRelative(Root);
		RootDirectory->fullPath = Root;
		ScanDirectory(RootDirectory);
	}

	std::string FileSystem::AbsoluteToRelative(const std::string& path)
	{
		const fs::path rootPath(Root);
		const fs::path absolutePath(path);
		return fs::relative(absolutePath, rootPath).generic_string();
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
		if (!DirectoryExists(subFolderPath))
		{
			MakeDirectory(subFolderPath);
		}

		return subFolderPath;
	}

	Ref<Directory> FileSystem::GetFileTree()
	{
		return RootDirectory;
	}

	Ref<File> FileSystem::GetFile(const std::string& path)
	{
		// Note, Might be broken on other platforms.
		auto splits = String::Split(path, '/');

		int currentDepth = -1;
		std::string currentDirName = ".";
		Ref<Directory> currentDirComparator = RootDirectory;
		while (currentDirName == currentDirComparator->name)
		{
			currentDepth++;
			currentDirName = splits[currentDepth];

			// Find next directory
			for (auto& d : currentDirComparator->Directories)
			{
				if (d->name == currentDirName)
				{
					currentDirComparator = d;
				}
			}

			// Find in files if can't find in directories.
			for (auto& f : currentDirComparator->Files)
			{
				if (f->GetName() == currentDirName)
				{
					return f;
				}
			}
		}

		return nullptr;
	}

	std::string FileSystem::GetFileNameFromPath(const std::string& path)
	{
		const auto& split = String::Split(path, '\\');
		return String::Split(split[split.size() - 1], '.')[0];
	}
	
}

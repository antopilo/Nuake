#include "OS.h"

#include "src/Window.h"
#include "Engine.h"

#ifdef NK_WIN

#define GLFW_EXPOSE_NATIVE_WIN32
#include <Windows.h>
#include <ShlObj.h>
#include <string.h>
#include <tchar.h>

#endif

#ifdef NK_LINUX
#include <gtk/gtk.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xlib.h>
#endif

#include "GLFW/glfw3.h"
#include "GLFW/glfw3native.h"


#include <chrono>
#include <imgui/imgui.h>

namespace Nuake {

	void OS::CopyToClipboard(const std::string& value)
	{
#ifdef NK_WIN
		auto glob = GlobalAlloc(GMEM_FIXED, 512);
		memcpy(glob, value.data(), value.size());
		OpenClipboard(glfwGetWin32Window(Window::Get()->GetHandle()));
		EmptyClipboard();
		SetClipboardData(CF_TEXT, glob);
		CloseClipboard();
#endif

#ifdef NK_LINUX

		glfwSetClipboardString(NULL, value.c_str());


#endif

	}

	std::string OS::GetFromClipboard()
	{
#ifdef NK_WIN
		OpenClipboard(nullptr);
		HANDLE hData = GetClipboardData(CF_TEXT);

		char* pszText = static_cast<char*>(GlobalLock(hData));
		std::string text(pszText);

		GlobalUnlock(hData);
		CloseClipboard();

		return text;
#endif

#ifdef NK_LINUX
		glfwSetClipboardString(NULL, "A string with words in it");
#endif

		return "";
	}

	int OS::GetTime()
	{
		return static_cast<int>(std::chrono::system_clock::now().time_since_epoch().count());
	}

	void OS::OpenIn(const std::string& filePath)
	{
#ifdef NK_WIN
		ShellExecuteA(nullptr, "open", filePath.c_str(), nullptr, nullptr, SW_SHOWDEFAULT);
#endif

#ifdef NK_LINUX
		std::string command = "xdg-open " + filePath;
		system(command.c_str());
#endif
	}

	int OS::RenameFile(const Ref<File>& file, const std::string& newName)
	{
		std::string extension = !String::EndsWith(newName, file->GetExtension().c_str()) ? file->GetExtension() : "";
		std::string newFilePath = file->GetParent()->fullPath + newName + extension;

		std::error_code resultError;
		std::filesystem::rename(file->GetAbsolutePath().c_str(), newFilePath.c_str(), resultError);
		return resultError.value() == 0;
	}

	int OS::RenameDirectory(const Ref<Directory>& dir, const std::string& newName)
	{
		std::string newDirPath = dir->Parent->fullPath + newName;

		std::error_code resultError;
		std::filesystem::rename(dir->fullPath.c_str(), newDirPath.c_str(), resultError);
		return resultError.value() == 0;
	}

	void OS::ShowInFileExplorer(const std::string& filePath)
	{
#ifdef NK_WIN
		ShellExecuteA(nullptr, "open", "explorer.exe", ("/select," + std::string(filePath)).c_str(), nullptr, SW_SHOWDEFAULT);
#endif

#ifdef NK_LINUX
		// Use xdg-open to open the directory containing the file
		std::string command = "xdg-open " + filePath.substr(0, filePath.find_last_of('/'));
		system(command.c_str());
#endif
	}

	void OS::OpenTrenchbroomMap(const std::string& filePath)
	{
#ifdef NK_WIN
		ShellExecuteA(nullptr, nullptr, Engine::GetProject()->TrenchbroomPath.c_str(), filePath.c_str(), nullptr, SW_SHOW);
#endif

#ifdef NK_LINUX
		std::string command = Engine::GetProject()->TrenchbroomPath + filePath.c_str();
		system(command.c_str());
#endif
	}

	std::string OS::GetConfigFolderPath()
	{
		std::string path;

#ifdef NK_WIN
		TCHAR appDataPath[64];
		// Get the path to the AppData folder
		if (SHGetFolderPath(NULL, CSIDL_LOCAL_APPDATA, NULL, 0, appDataPath) >= 0)
		{
			std::wstring wideString(appDataPath);
			path = std::string(wideString.begin(), wideString.end());
			// Now, 'appDataPath' contains the path to the AppData folder
			path.erase(std::remove_if(path.begin(), path.end(), [](char c)
				{
					return c == '\0';
				}), path.end());
			std::replace(path.begin(), path.end(), '\\', '/');
		}
		else
		{
			path = "";
		}
#endif

#ifdef NK_LINUX
		path = "~/.config/";
#endif

		return path;
	}

	void OS::OpenURL(const std::string& url)
	{
#ifdef NK_WIN
		ShellExecute(nullptr, nullptr, std::wstring(url.begin(), url.end()).c_str(), 0, 0, SW_SHOW);
#endif

#ifdef NK_LINUX
		std::string command = "xdg-open " + url;
		system(command.c_str());
#endif
	}
}
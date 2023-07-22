#include "OS.h"
#include "src/Window.h"

#define GLFW_EXPOSE_NATIVE_WIN32
#include "GLFW/glfw3.h"
#include "GLFW/glfw3native.h"

#include <Windows.h>
#include <chrono>

using namespace Nuake;

void OS::CopyToClipboard(const std::string& value)
{
	auto glob = GlobalAlloc(GMEM_FIXED, 512);
	memcpy(glob, value.data(), value.size());
	OpenClipboard(glfwGetWin32Window(Window::Get()->GetHandle()));
	EmptyClipboard();
	SetClipboardData(CF_TEXT, glob);
	CloseClipboard();
}

std::string OS::GetFromClipboard()
{
	OpenClipboard(nullptr);
	HANDLE hData = GetClipboardData(CF_TEXT);

	char* pszText = static_cast<char*>(GlobalLock(hData));
	std::string text(pszText);

	GlobalUnlock(hData);
	CloseClipboard();

	return text;
}

int OS::GetTime()
{
	return static_cast<int>(std::chrono::system_clock::now().time_since_epoch().count());
}

void OS::OpenIn(const std::string& filePath)
{
	ShellExecuteA(nullptr, "open", filePath.c_str(), nullptr, nullptr, SW_SHOWDEFAULT);
}

void OS::ShowInFileExplorer(const std::string& filePath)
{
	ShellExecuteA(nullptr, "open", "explorer.exe", ("/select," + std::string(filePath)).c_str(), nullptr, SW_SHOWDEFAULT);
}
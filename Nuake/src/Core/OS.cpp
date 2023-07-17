#include "OS.h"

#include <chrono>
#include <Windows.h>

using namespace Nuake;

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
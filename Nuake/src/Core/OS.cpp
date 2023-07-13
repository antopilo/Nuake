#include "OS.h"

#include <chrono>
#include "String.h"
#include <Windows.h>

using namespace Nuake;

int OS::GetTime() 
{
	return static_cast<int>(std::chrono::system_clock::now().time_since_epoch().count());
}

void OS::OpenInFileExplorer(const std::string& filePath)
{
	ShellExecuteA(nullptr, "open", "explorer.exe", ("/select," + std::string(filePath)).c_str(), nullptr, SW_SHOWDEFAULT);
}
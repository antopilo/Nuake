#pragma once

#include <string>

namespace Nuake
{
	class OS 
	{
	public:
		static int GetTime();
		static void OpenIn(const std::string& filePath);
		static void ShowInFileExplorer(const std::string& filePath);
	};
}

#pragma once

#include <string>


namespace Nuake
{
	class OS 
	{
	public:
		static int GetTime();
		static void OpenInFileExplorer(const std::string& filePath);
	};
}

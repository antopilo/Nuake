#pragma once

#include <string>

namespace Nuake
{
	class OS 
	{
	public:
		static int GetTime();
		static void ShowInFileExplorer(const std::string& filePath);
	};
}

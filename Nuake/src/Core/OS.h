#pragma once

#include <string>
#include "FileSystem.h"

namespace Nuake
{
	class OS 
	{
	public:
		static void CopyToClipboard(const std::string& value);
		static std::string GetFromClipboard();
		static int GetTime();
		static void OpenIn(const std::string& filePath);
		static int OS::RenameFile(const Ref<File>& file, const std::string& newName);
		static int OS::RenameDirectory(const Ref<Directory>& dir, const std::string& newName);
		static void ShowInFileExplorer(const std::string& filePath);
		static void OpenURL(const std::string& url);
	};
}

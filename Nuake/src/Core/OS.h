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
		static int RenameFile(const Ref<File>& file, const std::string& newName);
		static int RenameDirectory(const Ref<Directory>& dir, const std::string& newName);
		static void ShowInFileExplorer(const std::string& filePath);
		static void OpenTrenchbroomMap(const std::string& filePath);
		static void OpenURL(const std::string& url);
	};
}

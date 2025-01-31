#pragma once

#include <string>
#include "Nuake/FileSystem/FileSystem.h"

namespace Nuake
{
	class OS 
	{
	public:
		static void CopyToClipboard(const std::string& value);
		static std::string GetFromClipboard();
		static int GetTime();
		static void OpenIn(const std::string& filePath);
		static void ExecuteCommand(const std::string& command);
		static int RenameFile(const Ref<File>& file, const std::string& newName);
		static int RenameDirectory(const Ref<Directory>& dir, const std::string& newName);
		static void ShowInFileExplorer(const std::string& filePath);
		static void OpenTrenchbroomMap(const std::string& filePath);
		static void OpenURL(const std::string& url);
		static std::string GetConfigFolderPath();
		static std::string CompileSln(const std::string& slnPath);

		static int Subprocess(const std::string& command, std::string& out, std::string& err);
	};
}

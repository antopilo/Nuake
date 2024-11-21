#pragma once

#include "src/FileSystem/FileSystem.h"
#include "src/FileSystem/FileSystem.h"
#include <atomic>

class MapImporterWindow
{
public:
	MapImporterWindow() = default;
	~MapImporterWindow() = default;

	void Update();
	void Draw();

private:
	Ref<Nuake::File> m_MapToImport = nullptr;
	std::vector<std::string> m_DetectedWads;
	std::string m_OutputFile = "";
	std::vector<std::string> ScanUsedWads();
	std::map<std::string, std::string> m_WadToMaterialMap;
	std::atomic<bool> m_Working = false;
	std::atomic<int> m_Percentage;
	std::string GetTransformedWadPath(const std::string& path);
	
};
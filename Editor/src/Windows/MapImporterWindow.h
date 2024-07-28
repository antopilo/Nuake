#pragma once

#include <src/Core/FileSystem.h>
#include <src/Core/FileSystem.h>

class MapImporterWindow
{
public:
	MapImporterWindow() = default;
	~MapImporterWindow() = default;

	void Update();
	void Draw();

private:
	Ref<Nuake::File> m_MapToImport = nullptr;

	std::vector<std::string> ScanUsedWads();
};
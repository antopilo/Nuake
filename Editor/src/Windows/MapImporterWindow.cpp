#include "MapImporterWindow.h"
#include <imgui/imgui.h>
#include "../Misc/InterfaceFonts.h"
#include <src/UI/ImUI.h>
#include <src/Core/Logger.h>
#include <regex>

void MapImporterWindow::Draw()
{
	auto& io = ImGui::GetIO();
	ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f), ImGuiCond_FirstUseEver, ImVec2(0.5f, 0.5f));
	ImGui::SetNextWindowSizeConstraints(ImVec2(600, 400), ImVec2(1920, 1080));
	if (ImGui::Begin("Map Importer"))
	{
		ImGui::PushFont(FontManager::GetFont(Bold));
		ImGui::Text("What is this?");
		ImGui::PopFont();

		ImGui::TextWrapped("This imported will convert any Quake .map file into a .map that uses Nuake Material system.");

		ImGui::PushFont(FontManager::GetFont(Bold));
		ImGui::Text("1. Select a quake .map to import");
		ImGui::PopFont();

		std::string buttonLbl = "Empty";

		if (m_MapToImport)
		{
			buttonLbl = m_MapToImport->GetRelativePath();
		}

		using namespace Nuake;
		if (ImGui::Button(buttonLbl.c_str()))
		{
			std::string selectedFile = FileDialog::OpenFile("*.map");
			std::string relativePath = FileSystem::AbsoluteToRelative(selectedFile);
			if (FileSystem::FileExists(relativePath))
			{
				if (Ref<File> file = FileSystem::GetFile(relativePath); file)
				{
					m_MapToImport = file;
				}
			}
		}

		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("_Map"))
			{
				std::string fullPath = std::string(reinterpret_cast<char*>(payload->Data), 256);
				std::string relPath = FileSystem::AbsoluteToRelative(fullPath);
				if (FileSystem::FileExists(relPath))
				{
					if (Ref<File> file = FileSystem::GetFile(relPath); file)
					{
						m_MapToImport = file;
					}
				}
				else
				{
					Logger::Log("Failed to import map file. File doesn't exists", "map importer", CRITICAL);
				}
			}
			ImGui::EndDragDropTarget();
		}

		auto wads = ScanUsedWads();

		if (Nuake::UI::PrimaryButton("Export"))
		{
			m_MapToImport = nullptr;
		}
	}
	ImGui::End();
}

std::vector<std::string> MapImporterWindow::ScanUsedWads()
{
	if (!m_MapToImport)
	{
		return std::vector<std::string>();
	}

	using namespace Nuake;

	const std::string mapContent = m_MapToImport->Read();

	std::vector<std::string> usedWads;
	std::regex pattern("\"wad\"\\s*\"([^\"]+)\"");
	std::smatch match;

	// Search the input string using the regular expression
	if (std::regex_search(mapContent, match, pattern)) 
	{
		const std::string fullString = match[1].str();

		// Split multiple wads in case.
		auto separatedWads = String::Split(fullString, ';');
		for (auto& wad : separatedWads)
		{
			usedWads.push_back(wad);
		}
	}

	// If no match is found, return an empty string
	return usedWads;
}
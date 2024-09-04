#include "MapImporterWindow.h"

#include "../Misc/InterfaceFonts.h"

#include <Engine.h>
#include <src/Core/Logger.h>
#include <src/FileSystem/File.h>
#include <src/Threading/JobSystem.h>
#include <src/UI/ImUI.h>
#include <src/FileSystem/FileDialog.h>

#include <imgui/imgui.h>

#include <filesystem>
#include <fstream>
#include <sstream>
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
			if (!selectedFile.empty() && FileSystem::FileExists(relativePath))
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

		if (Nuake::UI::PrimaryButton("Scan WADs"))
		{
			m_MapToImport = nullptr;
		}

		//for (auto& w : ScanUsedWads())
		//{
		//	auto pathSplits = String::Split(std::string(w.begin(), w.end() - 4), '/');
		//	std::string WadName = pathSplits[std::size(pathSplits) - 1];
		//	std::string TargetDirectory = "/Materials/" + WadName + "/";
		//}

		if (m_Working)
		{
			ImGui::BeginDisabled();
		}

		if (Nuake::UI::PrimaryButton("Export"))
		{
			if (m_OutputFile.empty())
			{
				std::string outputFile = FileDialog::SaveFile("*.map");
				if (!String::EndsWith(outputFile, ".map"))
				{
					outputFile += ".map";
				}
				m_OutputFile = FileSystem::AbsoluteToRelative(outputFile);
			}

			m_Percentage = 0;

			std::string mapContent = m_MapToImport->Read();
			JobSystem::Get().Dispatch([this, mapContent]() {
				m_Working = true;
				std::string fileContent = mapContent;
				std::istringstream stream(fileContent);

				std::ostringstream modifiedMapContent;

				std::string pattern = R"(\(\s*-?\d+(\.\d+)?\s*-?\d+(\.\d+)?\s*-?\d+(\.\d+)?\s*\)\s*\(\s*-?\d+(\.\d+)?\s*-?\d+(\.\d+)?\s*-?\d+(\.\d+)?\s*\)\s*\(\s*-?\d+(\.\d+)?\s*-?\d+(\.\d+)?\s*-?\d+(\.\d+)?\s*\)\s*([^\s]+))";
				std::regex regexPattern(pattern);

				std::string line;

				int lines = std::count(fileContent.begin(), fileContent.end(), '\n');

				int lineEdited = 0;
				
				while (std::getline(stream, line))
				{
					if (String::BeginsWith(line, "// Game:"))
					{
						line = "// Game: " + String::RemoveWhiteSpace(Nuake::Engine::GetProject()->Name);
					}
					else
					{
						std::smatch match;
						if (std::regex_search(line, match, regexPattern))
						{
							// If a match is found, return the captured value
							std::string result = match[10].str();
							std::string result2 = match[0].str();
							result2.erase(result2.end() - result.length(), result2.end());

							std::string transformWad = GetTransformedWadPath(result);
							if (transformWad.empty())
							{
								Logger::Log("Couldn't find texture: " + result, "map importer", CRITICAL);
							}

							line = result2 + std::regex_replace(line, regexPattern, transformWad);
						}
					}

					modifiedMapContent << line << std::endl;
					lineEdited++;

					int newPercentage = ((float)lineEdited / (float)lines) * 100;
					if (newPercentage != m_Percentage)
					{
						m_Percentage.store(newPercentage, std::memory_order_relaxed);
					}
				}

				FileSystem::BeginWriteFile(m_OutputFile);
				FileSystem::WriteLine(modifiedMapContent.str());
				FileSystem::EndWriteFile();
				
			}, [this]() {
				m_Working = false;
			});

			m_MapToImport = nullptr;
		}

		if (m_Working)
		{
			ImGui::EndDisabled();
		}

		ImGui::SameLine();

		ImGui::TextUnformatted(m_OutputFile.c_str());

		if (ImGui::IsItemClicked())
		{
			std::string outputFile = FileDialog::SaveFile("*.map");
			if (!String::EndsWith(outputFile, ".map"))
			{
				outputFile += ".map";
			}

			m_OutputFile = FileSystem::AbsoluteToRelative(outputFile);
		}

		if (m_Working)
		{
			ImGui::SameLine();

			ImGui::TextUnformatted((std::string("Progress: ") + std::to_string(m_Percentage) + "%").c_str());
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

	m_DetectedWads.clear();

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

std::string MapImporterWindow::GetTransformedWadPath(const std::string& path)
{
	const std::string& baseTextureDir = "/textures/";

	using namespace Nuake;
	if (m_WadToMaterialMap.find(path) != m_WadToMaterialMap.end())
	{
		const std::string resolvedPath = m_WadToMaterialMap[path];
		Logger::Log("Map importer found matching material: " + resolvedPath, "map importer", VERBOSE);
		return resolvedPath;
		return resolvedPath;
	}

	std::regex backslash_pattern("\\\\");
	for (const auto& entry : std::filesystem::recursive_directory_iterator(FileSystem::Root + baseTextureDir))
	{
		// Check if the current entry is a regular file and matches the file name
		const std::string stem = String::ToLower(entry.path().stem().string());
		std::string upperInput = String::ToLower(path);

		upperInput = String::Split(upperInput, '/')[String::Split(upperInput, '/').size() - 1];

		std::string prefix = "";
		if (String::BeginsWith(upperInput, "*"))
		{
			prefix = "star_";
		}
		else if (String::BeginsWith(upperInput, "+"))
		{
			prefix = "plus_";
		}
		else if (String::BeginsWith(upperInput, "-"))
		{
			prefix = "minu_";
		}
		else if (String::BeginsWith(upperInput, "/"))
		{
			prefix = "divd_";
		}

		if (!prefix.empty())
		{
			upperInput = prefix + std::string(upperInput.begin() + 1, upperInput.end());
		}

		if (entry.is_regular_file() && stem == upperInput)
		{
			std::filesystem::path relativePath = std::filesystem::relative(entry.path(), FileSystem::Root + "/textures/");
			std::filesystem::path pathWithoutExtension = relativePath;
			pathWithoutExtension = pathWithoutExtension.parent_path(); // Remove the file name

			pathWithoutExtension /= relativePath.stem(); // Add the file name without extension
			Logger::Log("Map importer found matching material: " + pathWithoutExtension.string(), "map importer", VERBOSE);

			const std::string& foundMaterialPath = std::regex_replace(pathWithoutExtension.string(), backslash_pattern, "/");
			m_WadToMaterialMap[path] = foundMaterialPath;
			return foundMaterialPath;
		}
	}

	Logger::Log("Map importer failed to find material: " + path, "map importer", VERBOSE);
	return std::string();
}

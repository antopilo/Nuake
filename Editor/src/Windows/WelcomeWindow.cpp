#define IMGUI_DEFINE_MATH_OPERATORS
#include "WelcomeWindow.h"

#include <src/Vendors/imgui/imgui.h>
#include "../Misc/InterfaceFonts.h"

#include <Engine.h>
#include <src/FileSystem/FileDialog.h>
#include "src/FileSystem/FileSystem.h"
#include <src/Resource/Project.h>
#include <src/Core/Logger.h>
#include <src/Rendering/Textures/TextureManager.h>
#include <src/Rendering/Textures/Texture.h>
#include <src/Rendering/Vulkan/VulkanImage/VulkanImage.h>
#include <json/json.hpp>

#include <string>
#include <vector>

#include "EditorInterface.h"
#include "FileSystemUI.h"

#include <src/UI/ImUI.h>

namespace Nuake
{
	ProjectPreview::ProjectPreview(const std::string& path)
	{
		Path = path;
		ReadProjectFile();
	}

	void ProjectPreview::ReadProjectFile()
	{
		std::string projectFile = Nuake::FileSystem::ReadFile(Path);
		if (projectFile != "")
		{
			nlohmann::json projectJson = nlohmann::json::parse(projectFile);
			Name = projectJson["Name"];
			Description = projectJson["Description"];

			const std::string projectIconPath = Path + "/../icon.png";
			if (FileSystem::FileExists(projectIconPath, true))
			{
				//ProjectIcon = TextureManager::Get()->GetTexture(projectIconPath);
			}
			else
			{
				ProjectIcon = TextureManager::Get()->GetTexture2("Resources/Images/nuake-logo.png");
			}
		}
		else
		{
			Name = Path;
			Description = "Error read project file";
		}
	}

	json ProjectPreview::Serialize()
	{
		BEGIN_SERIALIZE();
		j["Path"] = this->Path;
		END_SERIALIZE();
	}

	bool ProjectPreview::Deserialize(const json& j) 
	{
		if (!j.contains("Path"))
			return false;

		Path = j["Path"];
		ReadProjectFile();
		return true;
	}

	WelcomeWindow::WelcomeWindow(Nuake::EditorInterface* editor) : _Editor(editor)
	{
		_Projects = std::vector<ProjectPreview>();

		_RecentProjectFilePath = (FileSystem::GetConfigFolderPath() + "recent.json");

		ParseRecentFile();

		if (_Projects.size() > 0)
		{
			SelectedProject = 0;
		}

		// Load Nuake logo
		_NuakeLogo = TextureManager::Get()->GetTexture2(NUAKE_LOGO_PATH);
	}

	void WelcomeWindow::LoadQueuedProject()
	{
		FileSystem::SetRootDirectory(FileSystem::GetParentPath(queuedProjectPath));
		auto project = Project::New();
		auto projectFileData = FileSystem::ReadFile(queuedProjectPath, true);
		try
		{
			project->Deserialize(nlohmann::json::parse(projectFileData));
			project->FullPath = queuedProjectPath;

			Engine::LoadProject(project);

			_Editor->filesystem->m_CurrentDirectory = Nuake::FileSystem::RootDirectory;
		}
		catch (std::exception exception)
		{
			Logger::Log("Error loading project: " + queuedProjectPath, "editor", CRITICAL);
			Logger::Log(exception.what());
			return;
		}

		queuedProjectPath = "";
		Engine::GetCurrentWindow()->SetTitle("Nuake Engine - Editing " + project->Name);
	}

	void WelcomeWindow::ImportProject(const std::string& file)
	{
		if (file != "" && String::EndsWith(file, ".project"))
		{
			// Prevent importing the same project twice in the list
			bool alreadyContainsProject = false;
			for (auto& p : _Projects)
			{
				if (p.Path == file)
				{
					alreadyContainsProject = true;
				}
			}

			if (!alreadyContainsProject)
			{
				_Projects.push_back(ProjectPreview(file));
				SaveRecentFile();
			}
		}
	}

	void WelcomeWindow::Draw()
	{
		if (!queuedRemovalPath.empty())
		{
			std::erase_if(_Projects, [&](ProjectPreview preview) 
				{
				return preview.Path == queuedRemovalPath;
				});

			queuedRemovalPath = "";
			SaveRecentFile();
		}

		// Make viewport fullscreen
		ImGuiViewport* viewport = ImGui::GetMainViewport();
		ImGui::SetNextWindowPos(viewport->Pos);
		ImGui::SetNextWindowSize(viewport->Size);
		ImGui::SetNextWindowViewport(viewport->ID);

		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(32.0f, 32.0f));
		if(ImGui::Begin("Welcome Screen", 0, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoResize))
		{
			// Draw Nuake logo
			{
				const Vector2 logoSize = _NuakeLogo->GetSize();
				const ImVec2 imguiSize = ImVec2(logoSize.x, logoSize.y);
				ImGui::Image((ImTextureID)_NuakeLogo->GetImGuiDescriptorSet(), imguiSize);
			}

			//ImGui::SameLine();

			std::string versionInfo = "pre-alpha ";
#ifdef NK_DEBUG
			versionInfo += "debug build";
#endif
#ifdef NK_DIST
			versionInfo += "dist build";
#endif
			ImGui::TextColored({1, 1, 1, 0.5}, versionInfo.c_str());
			// ImGui::TextColored({ 1, 1, 1, 0.5 }, "239d9479");
			// Add padding under logo
			ImGui::Dummy(ImVec2(10, 16));

			DrawRightControls();
			ImGui::SameLine();

			DrawRecentProjectsSection();

			ImGui::End();
		}
		ImGui::PopStyleVar(2);
	}

	void WelcomeWindow::DrawRecentProjectsSection()
	{
		// Recent projects section takes up 80% of the width
		ImVec2 projectsWindowSize = ImGui::GetContentRegionAvail();

		ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0, 0, 0, 0));
		ImGui::BeginChild("Projects", projectsWindowSize);
		{
			//ImGui::Dummy({ 4, 4 });
			//ImGui::SameLine();
			//{
			//	auto font = UIFont(Bold);
			//	ImGui::Text("Recent Projects");
			//}

			ImGui::PopStyleColor();
			for (uint32_t i = 0; i < std::size(_Projects); i++)
			{
				DrawProjectItem(i);
			}

			ImGui::Dummy({ 1, 4 });
			ImGui::SameLine();
			const float itemHeight = 60.0f;
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
			if (UI::SecondaryButton("Import or Drag a Project", Vector2(ImGui::GetContentRegionAvail().x - 8, itemHeight)))
			{
				const std::string path = FileDialog::OpenFile("Project file(.project)\0*.project\0");

				ImportProject(path);
			}
			ImGui::PopStyleColor();
		}
		ImGui::EndChild();
	}

	void WelcomeWindow::DrawProjectItem(const uint32_t itemIndex)
	{
		ImGui::Dummy({ 4, 4 });
		ImGui::SameLine();
		const ProjectPreview& project = _Projects[itemIndex];
		const uint32_t itemHeight = 56;
		const float cursorYStart = ImGui::GetCursorPosY();

		const std::string selectableName = "##" + std::to_string(itemIndex);
		const bool isSelected = SelectedProject == itemIndex;
		//ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(10.0f / 255.f, 182.0f / 255.f, 255.f / 255.f, 1.0f));

		ImDrawList* draw_list = ImGui::GetWindowDrawList();
		draw_list->ChannelsSplit(2);

		// Channel number is like z-order. Widgets in higher channels are rendered above widgets in lower channels.
		draw_list->ChannelsSetCurrent(1);

		//ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0, 0, 0, 0));
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 8.0f);
		ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.25, 0.25, 0.5, 0.0));
		ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4(0.25, 0.25, 0.5, 0.0));
		ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.25, 0.25, 0.5, 0.0));
		bool result = ImGui::Selectable(selectableName.c_str(), SelectedProject == itemIndex, ImGuiSelectableFlags_AllowOverlap | ImGuiSelectableFlags_AllowDoubleClick, ImVec2(ImGui::GetContentRegionAvail().x - 12, itemHeight));
		ImGui::PopStyleVar();

		if (ImGui::BeginPopupContextWindow(selectableName.c_str()))
		{
			if (ImGui::MenuItem("Remove from list"))
			{
				queuedRemovalPath = _Projects[itemIndex].Path;
			}

			ImGui::EndPopup();
		}


		if (result)
		{
			SelectedProject = itemIndex;
		}

		if (result && ImGui::IsMouseDoubleClicked(0))
		{
			SaveRecentFile();

			queuedProjectPath = _Projects[itemIndex].Path;
		}

		ImU32 color = IM_COL32(63, 63, 66, 128);
		if (isSelected)
		{
			color = IM_COL32(63, 69, 79, 255);
		}
		else if (ImGui::IsItemHovered())
		{
			color = IM_COL32(20, 20, 20, 128);
		}

		if (isSelected && ImGui::IsKeyPressed(ImGuiKey_Delete))
		{
			queuedRemovalPath = _Projects[SelectedProject].Path;
		}

		if (result)
		{
			SelectedProject = itemIndex;
		}
		//ImGui::PopStyleColor();

		draw_list->ChannelsSetCurrent(0);
		ImVec2 p_min = ImGui::GetItemRectMin();
		ImVec2 p_max = ImGui::GetItemRectMax();
		ImGui::GetWindowDrawList()->AddRectFilled(p_min, p_max, color, 4.0f);

		draw_list->ChannelsMerge();

		ImGui::PopStyleColor(3);

		const ImVec2 padding = ImVec2(16.0f, 0.0f);
		const ImVec2 iconSize = ImVec2(54, 54.0f);
		ImGui::SetCursorPos(ImVec2(padding.x / 2.0, padding.y / 2.0) + ImVec2(4, cursorYStart));

		if (project.ProjectIcon)
		{
			ImGui::Image((ImTextureID)project.ProjectIcon->GetImGuiDescriptorSet(), iconSize);
		}
		else
		{
			//ImGui::Image((void*)Nuake::TextureManager::Get()->GetTexture("Resources/Images/cube.png")->GetID(), iconSize, ImVec2(0, 1), ImVec2(1, 0));
		}

		ImGui::SameLine();
		ImGui::SetCursorPosX(padding.x + iconSize.x);

		ImGui::SetCursorPosY(cursorYStart + 8.0f);
		{
			UIFont boldfont = UIFont(Fonts::Normal);

			if (isSelected)
			{
				ImGui::TextColored(ImVec4(119.0f / 255.0f, 187.0f / 255.0f, 1, 255.0f), project.Name.c_str());
			}
			else
			{
				ImGui::Text(project.Name.c_str());
			}
		}

		ImGui::SetCursorPosY(cursorYStart + 28.f);
		{
			ImGui::SetCursorPosX(padding.x + iconSize.x);
			UIFont boldfont = UIFont(Fonts::Normal);

			if (isSelected)
			{
				ImGui::TextColored(ImVec4(119.0f / 255.0f, 187.0f / 255.0f, 1, 255.0f), (ICON_FA_INFO_CIRCLE + std::string(" ") + project.Description).c_str());
			}
			else
			{
				ImGui::TextColored(ImVec4(1, 1, 1, 0.4), (ICON_FA_INFO_CIRCLE + std::string(" ") + project.Description).c_str());
			}
		}

		ImGui::SetCursorPosY(cursorYStart + itemHeight + 4.0f);
		ImGui::Dummy({ 4, 2 });
	}

	void WelcomeWindow::DrawRightControls()
	{
		const float buttonHeight = 36.0f;
		ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0, 0, 0, 0));
		ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 16.0f);

		bool opened = ImGui::BeginChild("Controls", ImVec2(250.0f, ImGui::GetContentRegionAvail().y), false);
		if (opened)
		{
			ImGui::PopStyleVar();
			const ImVec2 buttonSize = ImVec2(ImGui::GetContentRegionAvail().x, buttonHeight);
			ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.0f);
			ImGui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign, ImVec2(0.14f, 0.5f));
			const std::string buttonLabel = std::string(ICON_FA_FOLDER_PLUS) + " New Project...";
			if (Nuake::UI::PrimaryButton(buttonLabel.c_str(), { buttonSize.x, buttonSize.y }))
			{
				_Editor->isCreatingNewProject = true;
			}
			ImGui::PopStyleVar(2);
			ImGui::Separator();

			ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.0f);
			ImGui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign, ImVec2(0.16f, 0.5f));
			const std::string buttonLabelOpen = std::string(ICON_FA_FOLDER_OPEN) + " Load Project";
				
			bool hasProjectSelected = false;
			if (SelectedProject < std::size(_Projects) && SelectedProject >= 0)
			{
				hasProjectSelected = true;
			}

			if (!hasProjectSelected)
			{
				ImGui::BeginDisabled(true);
			}

			if (Nuake::UI::SecondaryButton(buttonLabelOpen, { buttonSize.x, buttonSize.y }))
			{
				SaveRecentFile();

				queuedProjectPath = _Projects[SelectedProject].Path;;
			}

			if (!hasProjectSelected)
			{
				ImGui::EndDisabled();
			}

			ImGui::PopStyleVar(2);
			
			ImGui::Separator();

			ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.0f);
			ImGui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign, ImVec2(0.09f, 0.5f));
			const std::string buttonLabelDoc = std::string(ICON_FA_EXTERNAL_LINK_SQUARE_ALT) + " Documentation";
			if (Nuake::UI::SecondaryButton(buttonLabelDoc, { buttonSize.x, buttonSize.y }))
			{
				OS::OpenURL("https://docs.readthedocs.io/en/latest/index.html");
			}
			ImGui::PopStyleVar(2);
		}

		if (!opened)
		{
			ImGui::PopStyleVar();
		}

		ImGui::EndChild();
		ImGui::PopStyleColor();
	}

	void WelcomeWindow::ParseRecentFile()
	{
		if (!FileSystem::FileExists(_RecentProjectFilePath, true))
		{
			FileSystem::BeginWriteFile(_RecentProjectFilePath, true);
			FileSystem::WriteLine(_RecentProjectFileDefaultContent);
			FileSystem::EndWriteFile();
		}

		std::string content = Nuake::FileSystem::ReadFile(_RecentProjectFilePath);
		if (content != "")
		{
			nlohmann::json j = nlohmann::json::parse(content);
			for (auto& project : j["Projects"])
			{
				auto projectPreview = ProjectPreview();
				projectPreview.Deserialize(project);
				_Projects.push_back(projectPreview);
			}
		}
	}

	void WelcomeWindow::SaveRecentFile()
	{
		std::string str;
		BEGIN_SERIALIZE();

		for (uint32_t i = 0; i < std::size(_Projects); i++)
		{
			ProjectPreview& project = _Projects[i];
			j["Projects"][i] = project.Serialize();
		}

		FileSystem::BeginWriteFile(_RecentProjectFilePath, true);
		FileSystem::WriteLine(j.dump(4));
		FileSystem::EndWriteFile();
	}
}

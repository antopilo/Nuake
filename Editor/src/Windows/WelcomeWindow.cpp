#include "WelcomeWindow.h"

#include <src/Vendors/imgui/imgui.h>
#include "../Misc/InterfaceFonts.h"

#include <Engine.h>
#include <src/Core/FileSystem.h>
#include <src/Resource/Project.h>
#include <src/Core/Logger.h>
#include <src/Rendering/Textures/TextureManager.h>
#include <src/Rendering/Textures/Texture.h>

#include <json/json.hpp>

#include <string>
#include <vector>

#include "EditorInterface.h"
#include "FileSystemUI.h"

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
				ProjectIcon = TextureManager::Get()->GetTexture(projectIconPath);
			}
			else
			{
				ProjectIcon = TextureManager::Get()->GetTexture("resources/Images/nuake-logo.png");
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

	bool ProjectPreview::Deserialize(const std::string& str) 
	{
		BEGIN_DESERIALIZE(str);

		if (!j.contains("Path"))
			return false;

		Path = j["Path"];
		ReadProjectFile();
		return true;
	}

	WelcomeWindow::WelcomeWindow(Nuake::EditorInterface* editor) : _Editor(editor)
	{
		_Projects = std::vector<ProjectPreview>();

		ParseRecentFile();

		// Load Nuake logo
		_NuakeLogo = TextureManager::Get()->GetTexture(NUAKE_LOGO_PATH);
	}

	void WelcomeWindow::Draw()
	{
		// Make viewport fullscreen
		ImGuiViewport* viewport = ImGui::GetMainViewport();
		ImGui::SetNextWindowPos(viewport->GetWorkPos());
		ImGui::SetNextWindowSize(viewport->GetWorkSize());
		ImGui::SetNextWindowViewport(viewport->ID);

		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(32.0f, 32.0f));
		ImGui::Begin("Welcome Screen", 0, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoResize);
		{
			// Draw Nuake logo
			{
				const Vector2 logoSize = _NuakeLogo->GetSize();
				const ImVec2 imguiSize = ImVec2(logoSize.x, logoSize.y);
				ImGui::Image((ImTextureID)_NuakeLogo->GetID(), imguiSize, ImVec2(0, 1), ImVec2(1, 0));
			}

			// Add padding under logo
			ImGui::Dummy(ImVec2(10, 25));
			{
				UIFont boldfont = UIFont(Fonts::SubTitle);
				ImGui::Text("Open recent");
			}

			DrawRecentProjectsSection();
			ImGui::SameLine();
			DrawRightControls();
		}

		ImGui::End();
		ImGui::PopStyleVar();
		ImGui::PopStyleVar();
	}

	void WelcomeWindow::DrawRecentProjectsSection()
	{
		// Recent projects section takes up 80% of the width
		ImVec2 projectsWindowSize = ImGui::GetContentRegionAvail();
		projectsWindowSize.x *= 0.8f;

		ImGui::BeginChild("Projects", projectsWindowSize, true);
		{
			for (uint32_t i = 0; i < std::size(_Projects); i++)
			{
				DrawProjectItem(i);
			}

			const float itemHeight = 120.0f;
			if (ImGui::Button("Import an existing project", ImVec2(ImGui::GetContentRegionAvailWidth(), itemHeight)))
			{
				const std::string path = FileDialog::OpenFile("Project file |*.project");
				if (path != "" && String::EndsWith(path, ".project"))
				{
					// Prevent importing the same project twice in the list
					bool alreadyContainsProject = false;
					for (auto& p : _Projects)
					{
						if (p.Path == path)
						{
							alreadyContainsProject = true;
						}
					}

					if (!alreadyContainsProject)
					{
						_Projects.push_back(ProjectPreview(path));
					}
				}
			}
		}
		ImGui::EndChild();
	}

	void WelcomeWindow::DrawProjectItem(const uint32_t itemIndex)
	{
		const ProjectPreview& project = _Projects[itemIndex];
		const uint32_t itemHeight = 120;
		const float cursorYStart = ImGui::GetCursorPosY();

		const std::string selectableName = "##" + std::to_string(itemIndex);
		const bool isSelected = SelectedProject == itemIndex;
		if (ImGui::Selectable(selectableName.c_str(), isSelected, ImGuiSelectableFlags_AllowItemOverlap, ImVec2(ImGui::GetContentRegionAvailWidth(), itemHeight)))
		{
			SelectedProject = itemIndex;
		}

		const ImVec2 padding = ImVec2(25.0f, 20.0f);
		const ImVec2 iconSize = ImVec2(100, 100);
		ImGui::SetCursorPos(padding / 2.0 + ImVec2(0, cursorYStart));

		ImGui::Image((ImTextureID)project.ProjectIcon->GetID(), iconSize, ImVec2(0, 1), ImVec2(1, 0));
		ImGui::SameLine();
		ImGui::SetCursorPosX(padding.x + iconSize.x + padding.x);

		ImGui::SetCursorPosX(padding.x + iconSize.x + padding.x);
		ImGui::SetCursorPosY(cursorYStart + padding.y);
		{
			UIFont boldfont = UIFont(Fonts::LargeBold);
			ImGui::Text(project.Name.c_str());
		}

		ImGui::SetCursorPosY(cursorYStart + padding.y + 35.f);
		{
			ImGui::SetCursorPosX(padding.x + iconSize.x + padding.x);
			UIFont boldfont = UIFont(Fonts::Bold);
			ImGui::Text(project.Description.c_str());
		}

		ImGui::SetCursorPosY(cursorYStart + itemHeight);
	}

	void WelcomeWindow::DrawRightControls()
	{
		const float buttonHeight = 58.0f;
		ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0, 0, 0, 0));
		if (ImGui::BeginChild("Controls", ImGui::GetContentRegionAvail(), false))
		{
			const ImVec2 buttonSize = ImVec2(ImGui::GetContentRegionAvailWidth(), buttonHeight);
			if (ImGui::Button("Create a new project", buttonSize))
			{
				std::string selectedProject = FileDialog::SaveFile("Project file\0*.project");
					
				if (!selectedProject.empty())
				{
					if(!String::EndsWith(selectedProject, ".project"))
						selectedProject += ".project";
						
					auto backslashSplits = String::Split(selectedProject, '\\');
					auto fileName = backslashSplits[backslashSplits.size() - 1];

					std::string finalPath = String::Split(selectedProject, '.')[0];

					if (String::EndsWith(fileName, ".project"))
					{
						// We need to create a folder
						if (const auto& dirPath = finalPath;
							!std::filesystem::create_directory(dirPath))
						{
							// Should we continue?
							Logger::Log("Failed creating project directory: " + dirPath);
						}

						finalPath += "\\" + fileName;
					}

					auto project = Project::New(String::Split(fileName, '.')[0], "no description", finalPath);
					Engine::LoadProject(project);
					Engine::LoadScene(Scene::New());
					project->Save();

					auto projectPreview = ProjectPreview();
					projectPreview.Name = project->Name;
					projectPreview.Description = project->Description;
					projectPreview.Path = project->FullPath;

					_Projects.push_back(projectPreview);
				}
			}
			ImGui::Separator();

			if (SelectedProject != -1)
			{
				if (ImGui::Button("Open an existing Project", buttonSize))
				{
					assert(SelectedProject < std::size(_Projects));

					using namespace Nuake;

					SaveRecentFile();

					std::string projectPath = _Projects[SelectedProject].Path;
					FileSystem::SetRootDirectory(FileSystem::GetParentPath(projectPath));

					auto project = Project::New();
					auto projectFileData = FileSystem::ReadFile(projectPath, true);
					try 
					{
						project->Deserialize(projectFileData);
						project->FullPath = projectPath;

						Engine::LoadProject(project);

						_Editor->filesystem->m_CurrentDirectory = Nuake::FileSystem::RootDirectory;
					}
					catch (std::exception exception)
					{
						Logger::Log("Error loading project: " + projectPath, CRITICAL);
						Logger::Log(exception.what());
					}
						
					Engine::GetCurrentWindow()->SetTitle("Nuake Engine - Editing " + project->Name);
				}
			}
		}

		ImGui::EndChild();
		ImGui::PopStyleColor();
	}

	void WelcomeWindow::ParseRecentFile()
	{
		if (!FileSystem::FileExists(_RecentProjectFilePath, true))
		{
			FileSystem::BeginWriteFile(_RecentProjectFilePath);
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
				projectPreview.Deserialize(project.dump());
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

		FileSystem::BeginWriteFile(_RecentProjectFilePath);
		FileSystem::WriteLine(j.dump(4));
		FileSystem::EndWriteFile();
	}
}

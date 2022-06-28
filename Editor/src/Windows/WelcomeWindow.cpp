#include "WelcomeWindow.h"

#include <src/Vendors/imgui/imgui.h>
#include "../Misc/InterfaceFonts.h"

#include <src/Core/FileSystem.h>
#include <src/Resource/Project.h>
#include <Engine.h>
#include <src/Core/Logger.h>

#include <json/json.hpp>
#include <string>
#include <vector>
#include "EditorInterface.h"

#include "FileSystemUI.h"

#include <future>

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
	}

	void WelcomeWindow::Draw()
	{
		ImGuiViewport* viewport = ImGui::GetMainViewport();
		ImGui::SetNextWindowPos(viewport->GetWorkPos());
		ImGui::SetNextWindowSize(viewport->GetWorkSize());
		ImGui::SetNextWindowViewport(viewport->ID);

		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(32.0f, 32.0f));
		ImGui::Begin("Welcome Screen", 0, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoResize);
		{
			{
				UIFont boldfont = UIFont(Fonts::Title);

				std::string text = "Nuake Engine";
				auto windowWidth = ImGui::GetWindowSize().x;
				auto textWidth = ImGui::CalcTextSize(text.c_str()).x;

				ImGui::SetCursorPosX((windowWidth - textWidth) * 0.5f);
				ImGui::Text(text.c_str());
			}
			{
				UIFont boldfont = UIFont(Fonts::SubTitle);
				std::string text = "An IdTech inspired game engine";
				auto windowWidth = ImGui::GetWindowSize().x;
				auto textWidth = ImGui::CalcTextSize(text.c_str()).x;

				ImGui::SetCursorPosX((windowWidth - textWidth) * 0.5f);
				ImGui::Text(text.c_str());
			}
			ImGui::Separator();
			{
				UIFont boldfont = UIFont(Fonts::SubTitle);
				ImGui::Text("Projects recently opened");
			}

			ImVec2 projectsWindowSize = ImGui::GetContentRegionAvail();
			projectsWindowSize.x *= 0.6f;

			ImGui::BeginChild("Projects", projectsWindowSize, true);
			{
				const uint32_t itemHeight = 120;

				for (uint32_t i = 0; i < std::size(_Projects); i++)
				{
					ProjectPreview& project = _Projects[i];

					float cursorYStart = ImGui::GetCursorPosY();

					std::string selectableName = "##" + std::to_string(i);
					bool isSelected = SelectedProject == i;
					if (ImGui::Selectable(selectableName.c_str(), isSelected, ImGuiSelectableFlags_AllowItemOverlap, ImVec2(ImGui::GetContentRegionAvailWidth(), itemHeight)))
					{
						SelectedProject = i;
					}

					ImGui::SetCursorPosY(cursorYStart);
					{
						UIFont boldfont = UIFont(Fonts::LargeBold);
						ImGui::Text(project.Name.c_str());
					}

					{
						UIFont boldfont = UIFont(Fonts::Bold);
						ImGui::Text(project.Description.c_str());
					}

					ImGui::SetCursorPosY(cursorYStart + itemHeight);
				}

				if (ImGui::Button("Import an existing project", ImVec2(ImGui::GetContentRegionAvailWidth(), itemHeight)))
				{
					std::string path = FileDialog::OpenFile("*.project");
					if (path != "")
					{
						bool alreadyContainsProject = false;
						for (auto& p : _Projects)
						{
							if (p.Path == path)
								alreadyContainsProject = true;
						}

						if (!alreadyContainsProject)
						{
							_Projects.push_back(ProjectPreview(path));
						}
					}
				}
			}
			ImGui::EndChild();

			ImGui::SameLine();

			if (ImGui::BeginChild("Controls", ImGui::GetContentRegionAvail(), true))
			{
				ImVec2 buttonSize = ImVec2(ImGui::GetContentRegionAvailWidth(), 58);
				if (ImGui::Button("New Project", buttonSize))
				{
					std::string selectedProject = FileDialog::SaveFile("Project file\0*.project");
					if (selectedProject == "") // Hit cancel
						return;

					auto pathSplit = String::Split(selectedProject, '/');
					auto projectFileName = pathSplit[pathSplit.size() - 1];
					Ref<Project> project = Project::New(projectFileName, "no description", selectedProject + ".project");
					Engine::LoadProject(project);
					Engine::LoadScene(Scene::New());
					project->Save();

					auto projectPreview = ProjectPreview();
					projectPreview.Name = project->Name;
					projectPreview.Description = project->Description;
					projectPreview.Path = project->FullPath;

					_Projects.push_back(projectPreview);
				}
				ImGui::Separator();
				if (SelectedProject != -1)
				{
					if (ImGui::Button("Load Project", buttonSize))
					{
						assert(SelectedProject < std::size(_Projects));

						using namespace Nuake;

						SaveRecentFile();

						std::string projectPath = _Projects[SelectedProject].Path;
						FileSystem::SetRootDirectory(projectPath + "/../");

						auto project = Project::New();
						auto projectFileData = FileSystem::ReadFile(projectPath, true);
						try {
							project->Deserialize(projectFileData);
							project->FullPath = projectPath;
							Engine::LoadProject(project);
						}
						catch (std::exception exception)
						{
							Logger::Log("Error loading project: " + projectPath, CRITICAL);
							Logger::Log(exception.what());
						}
						
						Engine::GetCurrentWindow()->SetTitle("Nuake Engine - Editing " + project->Name);
					}

					if (ImGui::Button("Remove Project"))
					{
						_Projects.erase(_Projects.begin() + SelectedProject);
					}
				}
			}

			ImGui::EndChild();
		}

		ImGui::End();
		ImGui::PopStyleVar();
		ImGui::PopStyleVar();
	}

	void WelcomeWindow::ParseRecentFile()
	{
		if (!FileSystem::FileExists(_RecentProjectFilePath))
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

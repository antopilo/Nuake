#include "NewProjectWindow.h"

#include "imgui/imgui.h"
#include "../Misc/InterfaceFonts.h"

#include <src/Resource/FontAwesome5.h>
#include <src/Rendering/Textures/TextureManager.h>
#include <src/Rendering/Textures/Texture.h>
#include <src/Scene/Entities/ImGuiHelper.h>
#include <src/FileSystem/FileDialog.h>
#include "src/FileSystem/FileSystem.h"
#include <src/UI/ImUI.h>
#include <src/Core/Logger.h>
#include <src/Resource/Project.h>
#include <Engine.h>

#include "EditorInterface.h"
#include "FileSystemUI.h"

namespace Nuake
{
	NewProjectWindow::NewProjectWindow(EditorInterface* editor) :
		m_Editor(editor)
	{
	}

	void NewProjectWindow::Draw()
	{
		ImGuiViewport* viewport = ImGui::GetMainViewport();
		ImGui::SetNextWindowPos(viewport->Pos);
		ImGui::SetNextWindowSize(viewport->Size);
		ImGui::SetNextWindowViewport(viewport->ID);

		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(32.0f, 32.0f));
		if (ImGui::Begin("NewProjectScreen", 0, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoResize))
		{
			{
				ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0, 0, 0, 0));
				ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 16.0f);
				bool opened = ImGui::BeginChild("TemplatesView", ImVec2(ImGui::GetContentRegionAvail().x * 0.5f, ImGui::GetContentRegionAvail().y), false, ImGuiChildFlags_AlwaysUseWindowPadding);
				if (opened)
				{
					ImGui::PopStyleVar();

					UIFont boldFont(Fonts::SubTitle);
					ImGui::Text("Templates");
					ImGui::Dummy(ImVec2(32, 16));
				}

				if (!opened)
				{
					ImGui::PopStyleVar();
				}

				ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.1, 0.1, 0.1, 0.2f));
				ImGui::BeginChild("TemplateContainer", { ImGui::GetContentRegionAvail().x - 64.0f, ImGui::GetContentRegionAvail().y }, true, ImGuiChildFlags_AlwaysUseWindowPadding);
				{
					ImGui::Dummy({ 4, 4 });
					DrawProjectTemplate(0, "Minimal", "The bare minimum required to get started in Nuake.");
					ImGui::Dummy({ 4, 2 });
					DrawProjectTemplate(1, "Shooter", "A Simple first person shooter game.");
					ImGui::Dummy({ 4, 2 });
					DrawProjectTemplate(2, "Demo", "The Nuake demo project.");
				}
				ImGui::EndChild();
				ImGui::PopStyleColor();

				ImGui::EndChild();
				ImGui::PopStyleColor();
			}

			ImGui::SameLine();

			{
				ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0, 0, 0, 0));
				ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 16.0f);
				bool opened = ImGui::BeginChild("ProjectSetupView", ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y), false);
				if (opened)
				{
					ImGui::PopStyleVar();

					{
						UIFont boldFont(Fonts::SubTitle);
						ImGui::Text("Project Setup");
					}

					ImGui::Dummy(ImVec2(32, 16));

					static std::string projectTitle = "";
					static bool showTitleEmptyWarning = false;
					static bool showPathWarning = false;
					ImGui::Text("Title");
					if (showTitleEmptyWarning && projectTitle.empty())
					{
						ImGui::SameLine();
						ImGui::TextColored({ 1.0, 0.1, 0.1, 1.0 }, ICON_FA_EXCLAMATION_TRIANGLE);

						Nuake::UI::Tooltip("Title required");
					}

					ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
					ImGui::InputText("##ProjectTitle", &projectTitle);

					ImGui::Dummy(ImVec2(32, 16));

					ImGui::Text("Description");
					ImGui::TextColored(ImVec4(1, 1, 1, 0.4), "A description for your new project");
					static std::string description = "";
					ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
					ImGui::InputText("##ProjectDescription", &description);

					ImGui::Dummy(ImVec2(32, 16));

					ImGui::Text("Location");

					static std::string location = "";
					static bool isPathValid = Nuake::FileSystem::DirectoryExists(location, true);
					if ((showPathWarning && location.empty()) || (!location.empty() && !Nuake::FileSystem::DirectoryExists(location, true)))
					{
						ImGui::SameLine();
						ImGui::TextColored({ 1.0, 0.1, 0.1, 1.0 }, ICON_FA_EXCLAMATION_TRIANGLE);

						if (location.empty())
						{
							Nuake::UI::Tooltip("Path required");
						}
						else if (!isPathValid)
						{
							Nuake::UI::Tooltip("Path doesn't exist");
						}
					}

					ImGui::TextColored(ImVec4(1, 1, 1, 0.4), "A folder in which to create your new project");
					std::string finalLocation = "";
					static std::string projectFileName = "";

					ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - 40.0f);
					ImGui::InputText("##Location", &location); ImGui::SameLine();
					if (ImGui::Button((ICON_FA_FOLDER + std::string("##folderOpen")).c_str()))
					{
						std::string folderPath = Nuake::FileDialog::SaveFile("Project File\0*.project");
						folderPath = Nuake::String::ReplaceSlash(folderPath);


						if (!folderPath.empty())
						{
							auto splits = Nuake::String::Split(folderPath, '/');
							projectFileName = splits[splits.size() - 1];
							if (!Nuake::String::EndsWith(projectFileName, ".project"))
							{
								projectFileName += ".project";
							}

							location = "";

							for (int i = 0; i < splits.size() - 1; i++)
							{
								location += splits[i] + "/";
							}
						}
					}

					finalLocation = location;
					static std::string projectParentPath = location;
					projectParentPath = location;
					static bool placeInFolder = false;
					if (placeInFolder)
					{
						projectParentPath += projectTitle + "/";
					}

					finalLocation = projectParentPath + projectFileName;

					if (!location.empty())
					{
						ImGui::TextColored(ImVec4(1, 1, 1, 0.4), "Project will be created at under:");
						ImGui::TextColored(ImVec4(1, 1, 1, 0.4), (finalLocation).c_str());
					}

					ImGui::Dummy(ImVec2(32, 16));

					ImGui::Text("Other");
					ImGui::Checkbox("Place in empty folder", &placeInFolder);

					static bool generateCsharpProject = true;
					ImGui::Checkbox("Generate C# project", &generateCsharpProject);

					static ImVec4 primaryColor = { 97.0f / 255.0f, 0.0f, 1.0f, 1.0f};

					ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 8, 8 });
					ImGui::ColorEdit3("Accent color", &primaryColor.x, ImGuiColorEditFlags_NoInputs);
					ImGui::PopStyleVar();

					ImGui::Dummy({ 1, ImGui::GetContentRegionAvail().y - 42 });

					ImGui::Dummy({ ImGui::GetContentRegionAvail().x - 130, 38 }); ImGui::SameLine();
					if (Nuake::UI::PrimaryButton((ICON_FA_PLUS + std::string(" Create")).c_str(), Nuake::Vector2{ 120, 38 }, Nuake::Color(primaryColor.x, primaryColor.y, primaryColor.z, primaryColor.w)))
					{
						if (projectTitle.empty())
						{
							showTitleEmptyWarning = true;
						}
						else
						{
							showTitleEmptyWarning = false;
						}

						if (location.empty())
						{
							showPathWarning = true;
						}
						else if (Nuake::FileSystem::DirectoryExists(location, true))
						{
							showPathWarning = false;
						}

						if (!showTitleEmptyWarning && !showPathWarning)
						{
							// Create project
							if (Nuake::String::EndsWith(finalLocation, ".project"))
							{
								// We need to create a folder
								if (const auto& dirPath = projectParentPath; !std::filesystem::create_directory(dirPath))
								{
									// Should we continue?
									Nuake::Logger::Log("Failed creating project directory: " + dirPath);
								}
							}

							Nuake::FileSystem::SetRootDirectory(projectParentPath);
							auto project = Nuake::Project::New(projectTitle, description, finalLocation);
							Nuake::Engine::LoadProject(project);
							Nuake::Engine::SetCurrentScene(Nuake::Scene::New());
							project->Settings.PrimaryColor = Nuake::Color(primaryColor.x, primaryColor.y, primaryColor.z, primaryColor.w);
							project->Save();

							auto window = Nuake::Window::Get();
							window->SetDecorated(true);
							window->Maximize();

							m_Editor->filesystem->m_CurrentDirectory = Nuake::FileSystem::RootDirectory;

							m_HasCreatedProject = true;
						}
					}
				}

				if (!opened)
				{
					ImGui::PopStyleVar();
				}

				ImGui::EndChild();
				ImGui::PopStyleColor();
			}

			ImGui::End();
		}
		ImGui::PopStyleVar(2);
	}

	bool opened = true;
	void NewProjectWindow::DrawProjectTemplate(uint32_t i, const std::string& title, const std::string& description)
	{
		opened = true;
		ImGui::ShowDemoWindow(&opened);
		ImGui::Dummy({ 4, 4 });
		ImGui::SameLine();
		opened = true;
		const uint32_t itemHeight = 50;
		const float cursorYStart = ImGui::GetCursorPosY();

		const std::string selectableName = "##" + title;
		const bool isSelected = m_SelectedTemplate == i;
		//ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(10.0f / 255.f, 182.0f / 255.f, 255.f / 255.f, 1.0f));

		ImDrawList* draw_list = ImGui::GetWindowDrawList();
		draw_list->ChannelsSplit(2);

		// Channel number is like z-order. Widgets in higher channels are rendered above widgets in lower channels.
		draw_list->ChannelsSetCurrent(1);

		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 8.0f);
		ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.25, 0.25, 0.5, 0.0));
		ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4(0.25, 0.25, 0.5, 0.0));
		ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.25, 0.25, 0.5, 0.0));
		bool result = ImGui::Selectable(selectableName.c_str(), m_SelectedTemplate == i, ImGuiSelectableFlags_AllowOverlap | ImGuiSelectableFlags_AllowDoubleClick, ImVec2(ImGui::GetContentRegionAvail().x - 12, itemHeight));
		ImGui::PopStyleVar();
		if (result)
		{
			m_SelectedTemplate == i;
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

		draw_list->ChannelsSetCurrent(0);
		ImVec2 p_min = ImGui::GetItemRectMin();
		ImVec2 p_max = ImGui::GetItemRectMax();
		ImGui::GetWindowDrawList()->AddRectFilled(p_min, p_max, color, 4.0f);

		draw_list->ChannelsMerge();

		ImGui::PopStyleColor(3);

		if (result)
		{
			m_SelectedTemplate = i;
		}

		//ImGui::PopStyleColor();

		const ImVec2 padding = ImVec2(16.0f, 0.0f);
		const ImVec2 iconSize = ImVec2(50, 50.0f);
		ImGui::SetCursorPos(ImVec2(padding.x / 2.0, padding.y / 2.0) + ImVec2(4, cursorYStart));

		ImGui::Image((void*)Nuake::TextureManager::Get()->GetTexture("Resources/Images/cube.png")->GetID(), iconSize, ImVec2(0, 1), ImVec2(1, 0));

		ImGui::SameLine();
		ImGui::SetCursorPosX(padding.x + iconSize.x);

		ImGui::SetCursorPosY(cursorYStart + 8.0f);
		{
			UIFont boldfont = UIFont(Fonts::Normal);

			if (isSelected)
			{
				ImGui::TextColored(ImVec4(119.0f / 255.0f, 187.0f / 255.0f, 1, 255.0f), title.c_str());
			}
			else
			{
				ImGui::Text(title.c_str());
			}
		}

		ImGui::SetCursorPosY(cursorYStart + 28.f);
		{
			ImGui::SetCursorPosX(padding.x + iconSize.x);
			UIFont boldfont = UIFont(Fonts::Normal);

			if (isSelected)
			{
				ImGui::TextColored(ImVec4(119.0f / 255.0f, 187.0f / 255.0f, 1, 255.0f), (ICON_FA_INFO_CIRCLE + std::string(" ") + description).c_str());
			}
			else
			{
				ImGui::TextColored(ImVec4(1, 1, 1, 0.4), (ICON_FA_INFO_CIRCLE + std::string(" ") + description).c_str());
			}
		}

		ImGui::SetCursorPosY(cursorYStart + itemHeight + 4.0f);
	}
}


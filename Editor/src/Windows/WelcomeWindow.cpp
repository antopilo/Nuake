#include "WelcomeWindow.h"
#include <src/Vendors/imgui/imgui.h>

#include "../Misc/InterfaceFonts.h"
#include <string>
#include <vector>

void WelcomeWindow::Draw()
{
	std::vector<std::string> projects = {
		"C:/Dev/Nuake-DemoProject/test.project"
	};

	ImGuiViewport* viewport = ImGui::GetMainViewport();
	ImGui::SetNextWindowPos(viewport->GetWorkPos());
	ImGui::SetNextWindowSize(viewport->GetWorkSize());
	ImGui::SetNextWindowViewport(viewport->ID);

	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(32.0f, 32.0f));
	ImGui::Begin("AL:SDAL:SKd", 0, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoResize);
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

		int idx = 0;
		ImGui::BeginChild("Projects", projectsWindowSize, true);
		{
			for (int i = 0; i < 6; i++)
			{
				float cursorY = ImGui::GetCursorPosY();
				std::string selectableName = "##My new project" + std::to_string(i);
				ImGui::Selectable(selectableName.c_str(), false, 0, ImVec2(ImGui::GetContentRegionAvailWidth(), 100));
				ImGui::SetCursorPosY(cursorY);
				{
					UIFont boldfont = UIFont(Fonts::LargeBold);
					ImGui::Text("Name of project");
				}

				{
					UIFont boldfont = UIFont(Fonts::Bold);
					ImGui::Text("last modified on 10/20/2022");
				}
				{
					UIFont boldfont = UIFont(Fonts::Normal);
					ImGui::TextWrapped("Description lajsflkahjfklashklashgkASDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDlashglkashlkghalskghaslghaslkghaklsg");
					//ImGui::Text("Description lajsflkahjfklashklashgklashglkashlkghalskghaslghaslkghaklsg");
				}
				ImGui::SetCursorPosY(cursorY + 100);
			}
		}
		ImGui::EndChild();
	}

	ImGui::End();
	ImGui::PopStyleVar();
	ImGui::PopStyleVar();
}
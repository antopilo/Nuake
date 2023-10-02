#include "TrenchbroomConfiguratorWindow.h"

#include <Engine.h>
#include <src/UI/ImUI.h>
#include <src/Core/OS.h>
#include <src/Rendering/Textures/TextureManager.h>
#include <src/Resource/Prefab.h>

void TrenchbroomConfiguratorWindow::Update()
{

}

void TrenchbroomConfiguratorWindow::Draw()
{
	auto& io = ImGui::GetIO();
	ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f), ImGuiCond_FirstUseEver, ImVec2(0.5f, 0.5f));
	ImGui::SetNextWindowSizeConstraints(ImVec2(600, 400), ImVec2(1920, 1080));
	if (ImGui::Begin("Trenchbroom Configurator"))
	{
		ImGui::PushFont(FontManager::GetFont(Bold));
		ImGui::Text("What is this?");
		ImGui::PopFont();

		ImGui::TextWrapped("The trenchbroom configurator is where you define your entities visible in trenchbroom. You can either define point entities from prefabs, or brush entities from scripts.");

		ImGui::PushFont(FontManager::GetFont(Bold));
		ImGui::Text("Warning");
		ImGui::PopFont();

		ImGui::TextWrapped("Make sure you have set your trenchbroom executable path in the project settings by click on your .project file in the file browser.");
		
		using namespace Nuake;

		auto project = Engine::GetProject();
		ImVec2 childSize = ImVec2();
		childSize.x = ImGui::GetContentRegionAvail().x;
		childSize.y = (ImGui::GetContentRegionAvail().y) - (ImGui::GetFontSize() * 2.0f + (16.0f * 2.0f));
		if (ImGui::BeginTabBar("##tabs", ImGuiTabBarFlags_None))
		{
			if(ImGui::BeginTabItem("Point Entities"))
			{
				ImGui::BeginChild("PointEntities", childSize, true);
				auto avail = ImGui::GetContentRegionAvail();
				const ImVec2 buttonSize = ImVec2(120, 120);
				int amount = avail.x / 110;
				if (amount <= 0)
				{
					amount = 1;
				}

				if (ImGui::BeginTable("pointEntitiesUITable", amount))
				{

					uint32_t itemIndex = 0;
					for (const auto& pe : project->EntityDefinitionsFile->PointEntities)
					{
						const auto& prefab = pe.Prefab;
						if (!prefab.empty())
						{
							DrawPrefabItem(pe);
						}

						ImGui::TableNextColumn();

						itemIndex++;
					}

					ImGui::EndTable();
				}

				ImGui::EndChild();

				if (ImGui::BeginDragDropTarget())
				{
					if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("_Prefab"))
					{
						std::string fullPath = std::string(reinterpret_cast<char*>(payload->Data), 256);
						std::string relPath = FileSystem::AbsoluteToRelative(fullPath);

						// Make sure file exists
						if (FileSystem::FileExists(relPath))
						{
							Ref<Prefab> prefab = Prefab::New(relPath);
							FGDPointEntity newPointEntity;
							newPointEntity.Name = prefab->DisplayName;
							newPointEntity.Prefab = relPath;
							newPointEntity.Description = prefab->DisplayName.empty() ? "No description provided" : prefab->DisplayName;
							project->EntityDefinitionsFile->PointEntities.push_back(std::move(newPointEntity));
						}
						else
						{
							Logger::Log("Failed to import prefab file. File doesn't exists", "trenchbroom", CRITICAL);
						}
					}
					ImGui::EndDragDropTarget();
				}
				ImGui::EndTabItem();
			}

			if (ImGui::BeginTabItem("Brush Entities"))
			{
				ImGui::BeginChild("BushEntities", childSize, true);

				ImGui::EndChild();
				ImGui::EndTabItem();
			}

			ImGui::EndTabBar();
		}

		if (UI::PrimaryButton("Export"))
		{
			project->EntityDefinitionsFile->Export();
		}
	}
	ImGui::End();
}

void TrenchbroomConfiguratorWindow::DrawPrefabItem(const Nuake::FGDPointEntity& pointEntity)
{
    using namespace Nuake;

    ImGui::PushFont(FontManager::GetFont(Icons));
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.0f);
    const char* icon = ICON_FA_FOLDER;
    const std::string id = std::string("##") + pointEntity.Name;

    ImVec2 prevCursor = ImGui::GetCursorPos();
    ImVec2 prevScreenPos = ImGui::GetCursorScreenPos();
    const bool selected = ImGui::Selectable(id.c_str(), false, ImGuiSelectableFlags_AllowOverlap | ImGuiSelectableFlags_AllowDoubleClick, ImVec2(100, 150));
    
    if (ImGui::IsItemHovered())
        ImGui::SetTooltip(pointEntity.Prefab.c_str());


    ImGui::SetCursorPos(prevCursor);
    ImGui::Image((ImTextureID)Nuake::TextureManager::Get()->GetTexture("Resources/Images/folder_icon.png")->GetID(), ImVec2(100, 100), ImVec2(0, 1), ImVec2(1, 0));

    auto& imguiStyle = ImGui::GetStyle();

    ImVec2 startOffset = ImVec2(imguiStyle.CellPadding.x / 2.0f, 0);
    ImVec2 offsetEnd = ImVec2(startOffset.x + imguiStyle. CellPadding.x / 2.0f, imguiStyle.CellPadding.y / 2.0f);
    ImU32 rectColor = IM_COL32(255, 255, 255, 16);
    ImGui::GetWindowDrawList()->AddRectFilled(prevScreenPos + ImVec2(0, 100) - startOffset, prevScreenPos + ImVec2(100, 150) + offsetEnd, rectColor, 1.0f);

	ImU32 rectColor2 = UI::PrimaryCol;
	ImGui::GetWindowDrawList()->AddRectFilled(prevScreenPos + ImVec2(0, 100) - startOffset, prevScreenPos + ImVec2(100, 101) + offsetEnd, rectColor2, 0.0f);
    std::string visibleName = pointEntity.Name;
    const uint32_t MAX_CHAR_NAME = 35;
    if (visibleName.size() > MAX_CHAR_NAME)
    {
        visibleName = std::string(visibleName.begin(), visibleName.begin() + MAX_CHAR_NAME - 3) + "...";
    }

    ImGui::TextWrapped(visibleName.c_str());

    ImGui::SetCursorPosY(prevCursor.y + 150 - ImGui::GetTextLineHeight());
    ImGui::TextColored({ 1, 1, 1, 0.5f }, "Prefab");

    ImGui::PopStyleVar();


    if (ImGui::BeginPopup(visibleName.c_str()))
    {
        if (ImGui::MenuItem("Remove"))
        {
            
        }

        ImGui::Separator();

        if (ImGui::BeginMenu("Copy"))
        {
            if (ImGui::MenuItem("Prefab Path"))
            {
                OS::CopyToClipboard(FileSystem::RelativeToAbsolute(pointEntity.Prefab));
            }

            ImGui::EndPopup();
        }

        ImGui::Separator();

        if (ImGui::MenuItem("Show in File Explorer"))
        {
            OS::OpenIn(FileSystem::RelativeToAbsolute(pointEntity.Prefab));
        }

        ImGui::EndPopup();
    }

    ImGui::PopFont();
}
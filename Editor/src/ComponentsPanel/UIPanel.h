#pragma once
#include "ComponentPanel.h"

#include "src/FileSystem/FileSystem.h"
#include <src/Core/Maths.h>
#include <src/Scene/Components/UIComponent.h>
#include <src/Scene/Entities/ImGuiHelper.h>


class UIPanel : ComponentPanel
{
public:
    UIPanel() = default;
    ~UIPanel() = default;

    void Draw(Nuake::Entity entity) override
    {
        using namespace Nuake;

        if (!entity.HasComponent<UIComponent>())
            return;

        auto& component = entity.GetComponent<UIComponent>();
        BeginComponentTable(UI EMITTER, UIComponent);
        {
            {
                ImGui::Text("HTML File");
                ImGui::TableNextColumn();

                std::string path = component.UIFilePath;
                ImGui::Button(component.UIFilePath.c_str(), ImVec2(ImGui::GetContentRegionAvail().x, 0));
                if (ImGui::BeginDragDropTarget())
                {
                    if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("_UIFile"))
                    {
                        char* file = (char*)payload->Data;
                        std::string fullPath = std::string(file, 256);
                        path = Nuake::FileSystem::AbsoluteToRelative(fullPath);
                        component.UIFilePath = path;
                    }
                    ImGui::EndDragDropTarget();
                }

                ImGui::TableNextColumn();

                ComponentTableReset(component.UIFilePath, "");
            }
        }
        EndComponentTable();
    }
};
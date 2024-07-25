#pragma once
#include "ComponentPanel.h"

#include <src/Scene/Entities/ImGuiHelper.h>
#include <src/Scene/Components/NavMeshVolumeComponent.h>
#include <src/Core/Maths.h>

class NavMeshVolumePanel : ComponentPanel {

public:
    NavMeshVolumePanel() {}

    void Draw(Nuake::Entity entity) override
    {
        using namespace Nuake;

        if (!entity.HasComponent<NavMeshVolumeComponent>())
        {
            return;
        }

        auto& component = entity.GetComponent<NavMeshVolumeComponent>();
        BeginComponentTable(NAVMESH VOLUME, NavMeshVolumeComponent);
        {
            {
                CompononentPropertyName("Volume Bounds");
                ImGui::TableNextColumn();

                Vector3 volumeSize = component.VolumeSize;

                ImGuiHelper::DrawVec3("VolumeSize", &volumeSize);

                if (volumeSize.x > 0 && volumeSize.y > 0 && volumeSize.z > 0)
                {
                    component.VolumeSize = volumeSize;
                }

                ImGui::TableNextColumn();

                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1, 1, 1, 0));

                std::string resetLabel = std::string(ICON_FA_UNDO) + "##ResetVolumeSize";
                if (ImGui::Button(resetLabel.c_str()))
                {
                    volumeSize = { 1.0, 1.0, 1.0 };
                }

                ImGui::PopStyleColor();
            }
            ImGui::TableNextColumn();
            {
                CompononentPropertyName("Generate");

                ImGui::TableNextColumn();

                if (UI::SecondaryButton("Generate"))
                {

                }

                ImGui::TableNextColumn();

                // No reset button

                ImGui::TableNextColumn();
            }
        }
        EndComponentTable();
    }
};
#pragma once
#include "ComponentPanel.h"
#include <src/Scene/Components/CameraComponent.h>
#include "src/FileSystem/FileSystem.h"

class CameraPanel : ComponentPanel {

public:
    CameraPanel() {}

    void Draw(Nuake::Entity entity) override
    {
        if (!entity.HasComponent<Nuake::CameraComponent>())
            return;

        auto& component = entity.GetComponent<Nuake::CameraComponent>();
        BeginComponentTable(CAMERA, Nuake::CameraComponent);
        {
            {
                ImGui::Text("FOV");
                ImGui::TableNextColumn();

                ImGui::DragFloat("##Fov", &component.CameraInstance->Fov, 0.1f, 0.1f, 360.0f);
                ImGui::TableNextColumn();
                ComponentTableReset(component.CameraInstance->Fov, 88.0f);
            }
            ImGui::TableNextColumn();
            {
                ImGui::Text("Exposure");
                ImGui::TableNextColumn();

                ImGui::DragFloat("##Exposure", &component.CameraInstance->Exposure, 0.1f, 0.1f, 10.0f);
                ImGui::TableNextColumn();
                ComponentTableReset(component.CameraInstance->Fov, 1.0f);
            }
            ImGui::TableNextColumn();
            {
                ImGui::Text("Gamma");
                ImGui::TableNextColumn();

                ImGui::DragFloat("##Gamma", &component.CameraInstance->Gamma, 0.1f, 0.1f, 10.0f);
                ImGui::TableNextColumn();
                ComponentTableReset(component.CameraInstance->Fov, 2.2f);
            }
        }
        EndComponentTable();
    }
};
#pragma once
#include "ComponentPanel.h"
#include <Nuake/Scene/Components/CameraComponent.h>
#include "Nuake/FileSystem/FileSystem.h"

class CameraPanel {

public:
    static void Draw(Nuake::Entity& entity, entt::meta_any& componentInstance)
    {
        using namespace Nuake;
        
        Nuake::CameraComponent* componentPtr = componentInstance.try_cast<Nuake::CameraComponent>();
        if (componentPtr == nullptr)
        {
            return;
        }
        Nuake::CameraComponent& component = *componentPtr;
        
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
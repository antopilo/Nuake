#pragma once
#include "ComponentPanel.h"
#include <src/Scene/Components/CharacterControllerComponent.h>
#include <src/Core/FileSystem.h>

class CharacterControllerPanel : ComponentPanel {

public:
    CharacterControllerPanel() {}

    void Draw(Nuake::Entity entity) override
    {
        if (!entity.HasComponent<Nuake::CharacterControllerComponent>())
            return;

        auto& component = entity.GetComponent<Nuake::CharacterControllerComponent>();
        BeginComponentTable(CHARACTER CONTROLLER, Nuake::CharacterControllerComponent);
        {
            {
                ImGui::Text("Height");
                ImGui::TableNextColumn();

                ImGui::DragFloat("##Height", &component.Height, 0.01f, 0.1f, 100.0f);
                ImGui::TableNextColumn();
                ComponentTableReset(component.Height, 1.0f)
            }
            ImGui::TableNextColumn();
            {
                ImGui::Text("Radius");
                ImGui::TableNextColumn();

                ImGui::DragFloat("##Radius", &component.Radius, 0.01f, 0.1f, 10.0f);
				ImGui::TableNextColumn();
                ComponentTableReset(component.Radius, 0.25f)
            }
            ImGui::TableNextColumn();
            {
                ImGui::Text("Mass");
                ImGui::TableNextColumn();

                ImGui::DragFloat("##Mass", &component.Mass, 0.001f, 0.00001f, 10.0f);
                ImGui::TableNextColumn();
                ComponentTableReset(component.Mass, 0.001f)
            }
        }
        EndComponentTable()
    }
};
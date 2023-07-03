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
                ImGui::Text("Friction");
                ImGui::TableNextColumn();

                ImGui::DragFloat("##Friction", &component.Friction, 0.01f, 0.1f, 100.0f);
                ImGui::TableNextColumn();
                ComponentTableReset(component.Friction, 0.5f)
            }
            ImGui::TableNextColumn();
            {
                ImGui::Text("Max Slope Angle");
                ImGui::TableNextColumn();

                ImGui::DragFloat("##MaxSlopeAngle", &component.MaxSlopeAngle, 0.01f, 0.1f, 90.0f);
				ImGui::TableNextColumn();
                ComponentTableReset(component.MaxSlopeAngle, 0.45f)
            }
        }
        EndComponentTable()
    }
};
#pragma once
#include "ComponentPanel.h"
#include <src/Scene/Entities/ImGuiHelper.h>
#include <src/Scene/Components/TransformComponent.h>

class TransformPanel : ComponentPanel {

public:
	TransformPanel() {}

	void Draw(Nuake::Entity entity) override
	{
        Nuake::TransformComponent& component = entity.GetComponent<Nuake::TransformComponent>();
        BeginComponentTable(TRANSFORM, Nuake::TransformComponent);
        {
            {
                ImGui::Text("Translation");
                ImGui::TableNextColumn();

                ImGuiHelper::DrawVec3("Translation", &component.Translation);
                ImGui::TableNextColumn();

                ComponentTableReset(component.Translation, Nuake::Vector3(0, 0, 0));
            }
            ImGui::TableNextColumn();
            {
                ImGui::Text("Rotation");
                ImGui::TableNextColumn();

                //ImGuiHelper::DrawVec3("Rotation", &component.Rotation);
                ImGui::TableNextColumn();

                //ComponentTableReset(component.Rotation, Nuake::Vector3(0, 0, 0));
            }
            ImGui::TableNextColumn();
            {
                ImGui::Text("Scale");
                ImGui::TableNextColumn();

                ImGuiHelper::DrawVec3("Scale", &component.Scale);
                ImGui::TableNextColumn();

                ComponentTableReset(component.Scale, Nuake::Vector3(1, 1, 1));
            }
        }
        EndComponentTable();
	}
};
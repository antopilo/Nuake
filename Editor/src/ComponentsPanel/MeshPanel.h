#pragma once
#include "ComponentPanel.h"
#include <src/Scene/Entities/ImGuiHelper.h>
#include <src/Scene/Components/MeshComponent.h>

class MeshPanel : ComponentPanel {

public:
    MeshPanel() {}

    void Draw(Nuake::Entity entity) override
    {
        if (!entity.HasComponent<Nuake::MeshComponent>())
            return;

        Nuake::MeshComponent& component = entity.GetComponent<Nuake::MeshComponent>();
        BeginComponentTable(MESH, Nuake::MeshComponent);
        {
            ImGui::Text("Mesh");
            ImGui::TableNextColumn();

            std::string path = component.ModelPath;
            ImGui::Button(component.ModelPath.c_str(), ImVec2(ImGui::GetContentRegionAvail().x, 0));
            if (ImGui::BeginDragDropTarget())
            {
                if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("_Model"))
                {
                    char* file = (char*)payload->Data;
                    std::string fullPath = std::string(file, 256);
                    path = Nuake::FileSystem::AbsoluteToRelative(fullPath);

                    component.ModelPath = path;
                    component.LoadModel();
                }
                ImGui::EndDragDropTarget();
            }

            ImGui::TableNextColumn();
            ComponentTableReset(component.ModelPath, "");
        }
        EndComponentTable();
    }
};
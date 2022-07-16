#pragma once
#include "ComponentPanel.h"
#include <src/Scene/Entities/ImGuiHelper.h>
#include <src/Scene/Components/MeshComponent.h>

#include <src/Resource/ResourceLoader.h>
#include <src/Core/String.h>

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

            std::string label = std::to_string(component.ModelResource->ID);
            ImGui::Button(label.c_str(), ImVec2(ImGui::GetContentRegionAvail().x, 0));

            if (ImGui::BeginDragDropTarget())
            {
                if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("_Model"))
                {
                    char* file = (char*)payload->Data;
                    std::string fullPath = std::string(file, 256);
                    fullPath = Nuake::FileSystem::AbsoluteToRelative(fullPath);
                    
                    if (Nuake::String::EndsWith(fullPath, ".model"))
                    {

                    }
                    else
                    {
                        component.ModelPath = fullPath;
                        component.LoadModel();
                    }
                }
                ImGui::EndDragDropTarget();
            }

            ImGui::TableNextColumn();
            ComponentTableReset(component.ModelPath, "");
        }
        EndComponentTable();
    }
};
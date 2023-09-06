#pragma once
#include <src/Core/Core.h>
#include "ComponentPanel.h"
#include "ModelResourceInspector.h"

#include <src/Scene/Entities/ImGuiHelper.h>
#include <src/Scene/Components/SkinnedModelComponent.h>

#include <src/Resource/ResourceLoader.h>
#include <src/Core/String.h>

class SkinnedModelPanel : ComponentPanel
{
private:
    Scope<ModelResourceInspector> m_ModelInspector;
    bool m_Expanded = false;
    std::string m_QueuedModelPath;

public:
    SkinnedModelPanel()
    {
        CreateScope<ModelResourceInspector>();
    }

    void Draw(Nuake::Entity entity) override
    {
        using namespace Nuake;
        if (!entity.HasComponent<SkinnedModelComponent>())
            return;

        SkinnedModelComponent& component = entity.GetComponent<SkinnedModelComponent>();
        BeginComponentTable(SKINNED MESH, SkinnedModelComponent);
        {
            ImGui::Text("Model");
            ImGui::TableNextColumn();

            std::string label = "None";

            const bool isModelNone = component.ModelResource == nullptr;
            if (!isModelNone)
            {
                label = std::to_string(component.ModelResource->ID);
            }

            if (ImGui::Button(label.c_str(), ImVec2(ImGui::GetContentRegionAvail().x, 0)))
            {
            }

            if (m_Expanded)
            {
                m_ModelInspector->Draw();
            }

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
                        m_QueuedModelPath = fullPath;
                        ImGui::OpenPopup("Create Skeleton");
                    }
                }
                ImGui::EndDragDropTarget();
            }

            if (ImGui::BeginPopupModal("Create Skeleton", NULL, ImGuiWindowFlags_AlwaysAutoResize)) 
            {
                ImGui::SetItemDefaultFocus();
                ImGui::Text("Would you like to create the skeleton structure in the scene tree?");
                ImGui::Separator();

                if (ImGui::Button("OK", ImVec2(120, 0))) 
                { 
                    component.ModelPath = m_QueuedModelPath;
                    component.LoadModel();

                    Scene* scene = entity.GetScene();
                    scene->CreateSkeleton(entity);

                    ImGui::CloseCurrentPopup(); 
                }

                ImGui::SetItemDefaultFocus();
                ImGui::SameLine();

                if (ImGui::Button("Cancel", ImVec2(120, 0))) 
                { 
                    ImGui::CloseCurrentPopup(); 
                }

                ImGui::EndPopup();
            }

            ImGui::TableNextColumn();
            ComponentTableReset(component.ModelPath, "");
        }
        EndComponentTable();
    }
};
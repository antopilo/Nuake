#pragma once
#include <Nuake/Core/Core.h>
#include "ComponentPanel.h"
#include "ModelResourceInspector.h"

#include <Nuake/Scene/Entities/ImGuiHelper.h>
#include <Nuake/Scene/Components/MeshCollider.h>

#include <Nuake/Resource/ResourceLoader.h>
#include <Nuake/Core/String.h>

class MeshColliderPanel : ComponentPanel
{
public:
    MeshColliderPanel()
    {
        CreateScope<ModelResourceInspector>();
    }

    static void Draw(Nuake::Entity& entity, entt::meta_any& componentInstance)
    {
        using namespace Nuake;
        
        Nuake::MeshColliderComponent* componentPtr = componentInstance.try_cast<Nuake::MeshColliderComponent>();
        if (componentPtr == nullptr)
        {
            return;
        }
        Nuake::MeshColliderComponent& component = *componentPtr;
        
        BeginComponentTable(MESH, MeshColliderComponent);
        {
            ImGui::Text("Model");
            ImGui::TableNextColumn();

            int editorValue = component.SubMesh;
            ImGui::DragInt("##Submesh", &editorValue, 0.1f, 0, 255);
            component.SubMesh = editorValue;

            ImGui::TableNextColumn();
            ComponentTableReset(component.SubMesh, 0);
        }
        EndComponentTable();
    }
};
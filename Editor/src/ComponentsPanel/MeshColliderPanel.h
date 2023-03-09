#pragma once
#include <src/Core/Core.h>
#include "ComponentPanel.h"
#include "ModelResourceInspector.h"

#include <src/Scene/Entities/ImGuiHelper.h>
#include <src/Scene/Components/MeshCollider.h>

#include <src/Resource/ResourceLoader.h>
#include <src/Core/String.h>

class MeshColliderPanel : ComponentPanel {
private:
    Scope<ModelResourceInspector> _modelInspector;
    bool _expanded = false;
public:
    MeshColliderPanel()
    {
        CreateScope<ModelResourceInspector>();
    }

    void Draw(Nuake::Entity entity) override
    {
        using namespace Nuake;
        if (!entity.HasComponent<MeshColliderComponent>())
            return;

        MeshColliderComponent& component = entity.GetComponent<MeshColliderComponent>();
        BeginComponentTable(MESH, MeshColliderComponent);
        {
            ImGui::Text("Model");
            ImGui::TableNextColumn();

            ImGui::DragInt("##Submesh", &component.SubMesh, 0, 255);
            ImGui::TableNextColumn();
            ComponentTableReset(component.SubMesh, 0);
        }
        EndComponentTable();
    }
};
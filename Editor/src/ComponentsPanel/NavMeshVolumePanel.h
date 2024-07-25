#pragma once
#include "ComponentPanel.h"

#include <src/Scene/Entities/ImGuiHelper.h>
#include <src/Scene/Components/NavMeshVolumeComponent.h>
#include <src/Core/Maths.h>
#include <src/AI/RecastConfig.h>

class NavMeshVolumePanel : ComponentPanel {

public:
    NavMeshVolumePanel() {}

    void Draw(Nuake::Entity entity) override
    {
        using namespace Nuake;

        if (!entity.HasComponent<NavMeshVolumeComponent>())
        {
            return;
        }

        auto& component = entity.GetComponent<NavMeshVolumeComponent>();
        BeginComponentTable(NAVMESH VOLUME, NavMeshVolumeComponent);
        {
            {
                CompononentPropertyName("Volume Bounds");
                ImGui::TableNextColumn();

                Vector3 volumeSize = component.VolumeSize;

                ImGuiHelper::DrawVec3("VolumeSize", &volumeSize);

                if (volumeSize.x > 0 && volumeSize.y > 0 && volumeSize.z > 0)
                {
                    component.VolumeSize = volumeSize;
                }

                ImGui::TableNextColumn();

                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1, 1, 1, 0));

                std::string resetLabel = std::string(ICON_FA_UNDO) + "##ResetVolumeSize";
                if (ImGui::Button(resetLabel.c_str()))
                {
                    volumeSize = { 1.0, 1.0, 1.0 };
                }

                ImGui::PopStyleColor();
            }
            ImGui::TableNextColumn();
            {
                CompononentPropertyName("Only include map");
                ImGui::TableNextColumn();

                Vector3 volumeSize = component.VolumeSize;

                UI::CheckBox("##onlyIncludeMapGeo", component.OnlyIncludeMapGeometry);

                ImGui::TableNextColumn();

                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1, 1, 1, 0));
                std::string resetLabel = std::string(ICON_FA_UNDO) + "##ResetOnlyIncludeGeo";
                if (ImGui::Button(resetLabel.c_str()))
                {
                    component.OnlyIncludeMapGeometry = true;
                }

                ImGui::PopStyleColor();
            }
            ImGui::TableNextColumn();
            {
                CompononentPropertyName("Generate");

                ImGui::TableNextColumn();

                if (UI::SecondaryButton("Generate"))
                {
                    if (component.OnlyIncludeMapGeometry)
                    {
                        auto view = Engine::GetCurrentScene()->m_Registry.view<TransformComponent, QuakeMapComponent>();
                        for (auto e : view)
                        {
                            auto [transform, map] = view.get<TransformComponent, QuakeMapComponent>(e);

                            for (auto& b : map.m_Brushes)
                            {
                                if (b.HasComponent<ModelComponent>())
                                {
                                    TransformComponent& transformComponent = b.GetComponent<TransformComponent>();
                                    for (auto& mesh : b.GetComponent<ModelComponent>().ModelResource->GetMeshes())
                                    {
                                        Nuake::NavManager::Get().PushMesh(mesh, transformComponent.GetGlobalTransform());
                                    }
                                }
                            }

                            NavManager::Get().BuildNavMesh(RecastConfig(component));
                        }
                    }
                }

                ImGui::TableNextColumn();

                // No reset button

                ImGui::TableNextColumn();
            }
        }
        EndComponentTable();
    }
};
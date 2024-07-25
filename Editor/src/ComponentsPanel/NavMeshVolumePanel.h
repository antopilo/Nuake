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
                CompononentPropertyName("Cell Size");
                ImGui::TableNextColumn();

                UI::FloatSlider("##cellsize", component.CellSize, 0.01f);

                ImGui::TableNextColumn();

                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1, 1, 1, 0));
                std::string resetLabel = std::string(ICON_FA_UNDO) + "##resetcellsize";
                if (ImGui::Button(resetLabel.c_str()))
                {
                    component.CellSize = 0.2f;
                }

                ImGui::PopStyleColor();
            }

            ImGui::TableNextColumn();
            {
                CompononentPropertyName("Cell Height");
                ImGui::TableNextColumn();

                UI::FloatSlider("##cellHeight", component.CellHeight, 0.01f);

                ImGui::TableNextColumn();

                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1, 1, 1, 0));
                std::string resetLabel = std::string(ICON_FA_UNDO) + "##resetcellheight";
                if (ImGui::Button(resetLabel.c_str()))
                {
                    component.CellHeight = 0.2f;
                }

                ImGui::PopStyleColor();
            }

            ImGui::TableNextColumn();
            {
                CompononentPropertyName("Tile Size");
                ImGui::TableNextColumn();

                UI::FloatSlider("##tilesize", component.TileSize, 0.01f);

                ImGui::TableNextColumn();

                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1, 1, 1, 0));
                std::string resetLabel = std::string(ICON_FA_UNDO) + "##resettilesize";
                if (ImGui::Button(resetLabel.c_str()))
                {
                    component.TileSize = 10.0f;
                }

                ImGui::PopStyleColor();
            }

            ImGui::TableNextColumn();
            {
                CompononentPropertyName("Walkable Slope Angle");
                ImGui::TableNextColumn();

                UI::FloatSlider("##walkableslopeAngle", component.WalkableSlopeAngle, 1.0f);

                ImGui::TableNextColumn();

                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1, 1, 1, 0));
                std::string resetLabel = std::string(ICON_FA_UNDO) + "##resetwalkableslopeangle";
                if (ImGui::Button(resetLabel.c_str()))
                {
                    component.WalkableSlopeAngle = 45.0f;
                }

                ImGui::PopStyleColor();
            }

            ImGui::TableNextColumn();
            {
                CompononentPropertyName("Walkable Height");
                ImGui::TableNextColumn();

                UI::FloatSlider("##WalkableHeight", component.WalkableHeight, 0.1f);

                ImGui::TableNextColumn();

                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1, 1, 1, 0));
                std::string resetLabel = std::string(ICON_FA_UNDO) + "##resetWalkableHeight";
                if (ImGui::Button(resetLabel.c_str()))
                {
                    component.WalkableHeight = 1.0f;
                }

                ImGui::PopStyleColor();
            }

            ImGui::TableNextColumn();
            {
                CompononentPropertyName("Walkable Radius");
                ImGui::TableNextColumn();

                UI::FloatSlider("##WalkableRadius", component.WalkableRadius, 0.1f);

                ImGui::TableNextColumn();

                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1, 1, 1, 0));
                std::string resetLabel = std::string(ICON_FA_UNDO) + "##resetWalkableRadius";
                if (ImGui::Button(resetLabel.c_str()))
                {
                    component.WalkableRadius = 1.0f;
                }

                ImGui::PopStyleColor();
            }

            ImGui::TableNextColumn();
            {
                CompononentPropertyName("Walkable Climb");
                ImGui::TableNextColumn();

                UI::FloatSlider("##WalkableClimb", component.WalkableClimb, 0.1f);

                ImGui::TableNextColumn();

                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1, 1, 1, 0));
                std::string resetLabel = std::string(ICON_FA_UNDO) + "##resetWalkableClimb";
                if (ImGui::Button(resetLabel.c_str()))
                {
                    component.WalkableClimb = 1.0f;
                }

                ImGui::PopStyleColor();
            }

            ImGui::TableNextColumn();
            {
                CompononentPropertyName("Max Simplification Error");
                ImGui::TableNextColumn();

                UI::FloatSlider("##MaxSimplificationError", component.MaxSimplificationError, 0.1f);

                ImGui::TableNextColumn();

                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1, 1, 1, 0));
                std::string resetLabel = std::string(ICON_FA_UNDO) + "##resetMaxSimplificationError";
                if (ImGui::Button(resetLabel.c_str()))
                {
                    component.MaxSimplificationError = 1.3f;
                }

                ImGui::PopStyleColor();
            }

            ImGui::TableNextColumn();
            {
                CompononentPropertyName("Min Region Area");
                ImGui::TableNextColumn();

                UI::FloatSlider("##MinRegionArea", component.MinRegionArea, 0.1f);

                ImGui::TableNextColumn();

                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1, 1, 1, 0));
                std::string resetLabel = std::string(ICON_FA_UNDO) + "##resetMinRegionArea";
                if (ImGui::Button(resetLabel.c_str()))
                {
                    component.MinRegionArea = 4.f;
                }

                ImGui::PopStyleColor();
            }

            ImGui::TableNextColumn();
            {
                CompononentPropertyName("Merge Region Area");
                ImGui::TableNextColumn();

                UI::FloatSlider("##MergeRegionArea", component.MergeRegionArea, 0.1f);

                ImGui::TableNextColumn();

                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1, 1, 1, 0));
                std::string resetLabel = std::string(ICON_FA_UNDO) + "##resetMergeRegionArea";
                if (ImGui::Button(resetLabel.c_str()))
                {
                    component.MergeRegionArea = 10.f;
                }

                ImGui::PopStyleColor();
            }

            ImGui::TableNextColumn();
            {
                CompononentPropertyName("Max Edge Length");
                ImGui::TableNextColumn();

                UI::FloatSlider("##MaxEdgeLength", component.MaxEdgeLength, 0.1f);

                ImGui::TableNextColumn();

                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1, 1, 1, 0));
                std::string resetLabel = std::string(ICON_FA_UNDO) + "##resetMaxEdgeLength";
                if (ImGui::Button(resetLabel.c_str()))
                {
                    component.MaxEdgeLength = 12.f;
                }

                ImGui::PopStyleColor();
            }

            ImGui::TableNextColumn();
            {
                CompononentPropertyName("MaxVertsPerPoly");
                ImGui::TableNextColumn();

                UI::FloatSlider("##MaxVertsPerPoly", component.MaxVertsPerPoly, 2.f);

                ImGui::TableNextColumn();

                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1, 1, 1, 0));
                std::string resetLabel = std::string(ICON_FA_UNDO) + "##resetMaxVertsPerPoly";
                if (ImGui::Button(resetLabel.c_str()))
                {
                    component.MaxVertsPerPoly = 6.f;
                }

                ImGui::PopStyleColor();
            }

            ImGui::TableNextColumn();
            {
                CompononentPropertyName("Detail Sample Distance");
                ImGui::TableNextColumn();

                UI::FloatSlider("##DetailSampleDistance", component.DetailSampleDistance, 0.1f);

                ImGui::TableNextColumn();

                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1, 1, 1, 0));
                std::string resetLabel = std::string(ICON_FA_UNDO) + "##resetDetailSampleDistance";
                if (ImGui::Button(resetLabel.c_str()))
                {
                    component.DetailSampleDistance = 2.f;
                }

                ImGui::PopStyleColor();
            }

            ImGui::TableNextColumn();
            {
                CompononentPropertyName("Detail Sample Max Error");
                ImGui::TableNextColumn();

                UI::FloatSlider("##DetailsampleMaxError", component.DetailsampleMaxError, 0.1f);

                ImGui::TableNextColumn();

                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1, 1, 1, 0));
                std::string resetLabel = std::string(ICON_FA_UNDO) + "##resetDetailsampleMaxError";
                if (ImGui::Button(resetLabel.c_str()))
                {
                    component.DetailsampleMaxError = 1.f;
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
                            for (auto& brush : map.m_Brushes)
                            {
                                if (!brush.HasComponent<ModelComponent>())
                                {
                                    continue;
                                }

                                auto& transformComponent = brush.GetComponent<TransformComponent>();
                                auto& modelComponent = brush.GetComponent<ModelComponent>();
                                for (auto& mesh : modelComponent.ModelResource->GetMeshes())
                                {
                                    Nuake::NavManager::Get().PushMesh(mesh, transformComponent.GetGlobalTransform());
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
#pragma once
#include "ComponentPanel.h"
#include "src/Scene/Components/LightComponent.h"

class LightPanel :ComponentPanel {

public:
	LightPanel() { }

	void Draw(Nuake::Entity entity) override
	{
        if (!entity.HasComponent<Nuake::LightComponent>())
            return;

        Nuake::LightComponent& component = entity.GetComponent<Nuake::LightComponent>();

        BeginComponentTable(LIGHT, Nuake::LightComponent);
        {
            {
                ImGui::Text("Color");

                ImGui::TableNextColumn();
                ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
                ImGui::ColorEdit3("##lightcolor", &component.Color.r, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel);
                ImGui::PopItemWidth();

                ImGui::TableNextColumn();
                ComponentTableReset(component.Color, Nuake::Vector3(1, 1, 1));
            }
            ImGui::TableNextColumn();
            {
                ImGui::Text("Strength");

                ImGui::TableNextColumn();
                ImGui::SliderFloat("Strength", &component.Strength, 0.0f, 100.0f);

                ImGui::TableNextColumn();
                ComponentTableReset(component.Strength, 10.f);
            }
            ImGui::TableNextColumn();
            {
                ImGui::Text("Cast Shadows");
                ImGui::TableNextColumn();

                // Using the SetCastShadows allows to create Framebuffers.
                bool castShadows = component.CastShadows;
                ImGui::Checkbox("##CastShadows", &castShadows);
                if (castShadows != component.CastShadows)
                    component.SetCastShadows(castShadows);

                ImGui::TableNextColumn();
                ComponentTableReset(component.CastShadows, false);
            }
            ImGui::TableNextColumn();
            {
                ImGui::Text("Type");
                ImGui::TableNextColumn();

                const char* lightTypes[] = { "Directional", "Point", "Spot" };
                ComponentDropDown(lightTypes, Nuake::LightType, component.Type)
                
                ImGui::TableNextColumn();
                ComponentTableReset(component.Type, Nuake::LightType::Point);
            }
            ImGui::TableNextColumn();
            if (component.Type == Nuake::LightType::Directional)
            {
                {
                    ImGui::Text("Is Volumetric");

                    ImGui::TableNextColumn();
                    ImGui::Checkbox("##Volumetric", &component.IsVolumetric);

                    ImGui::TableNextColumn();
                    ComponentTableReset(component.IsVolumetric, false);
                }
                ImGui::TableNextColumn();
                {
                    ImGui::Text("Sync Direction with sky");

                    ImGui::TableNextColumn();
                    ImGui::Checkbox("##SyncSky", &component.SyncDirectionWithSky);

                    ImGui::TableNextColumn();
                    ComponentTableReset(component.SyncDirectionWithSky, false);

                    // Light direction is only useful if it is not overriden by the procedural sky direction
                    if (!component.SyncDirectionWithSky)
                    {
                        ImGui::TableNextColumn();
                        {
                            ImGui::Text("Direction");

                            ImGui::TableNextColumn();
                            ImGuiHelper::DrawVec3("Direction", &component.Direction);

                            ImGui::TableNextColumn();
                            ComponentTableReset(component.Direction, Nuake::Vector3(0, -1, 0));
                        }
                    }
                }
            }
        }  
        EndComponentTable();
	}
};
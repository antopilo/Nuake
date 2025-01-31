#pragma once
#include "ComponentPanel.h"
#include <Nuake/Scene/Components/CharacterControllerComponent.h>
#include "Nuake/FileSystem/FileSystem.h"

class CharacterControllerPanel
{
public:
    static void Draw(Nuake::Entity& entity, entt::meta_any& componentInstance)
    {
        using namespace Nuake;
        
        Nuake::CharacterControllerComponent* componentPtr = componentInstance.try_cast<Nuake::CharacterControllerComponent>();
        if (componentPtr == nullptr)
        {
            return;
        }
        Nuake::CharacterControllerComponent& component = *componentPtr;
        
        BeginComponentTable(CHARACTER CONTROLLER, Nuake::CharacterControllerComponent);
        {
            {
                ImGui::Text("Friction");
                ImGui::TableNextColumn();

                ImGui::DragFloat("##Friction", &component.Friction, 0.00f, 0.1f, 100.0f);
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
            ImGui::TableNextColumn();
            {
                ImGui::Text("Auto stepping");
                ImGui::TableNextColumn();

                ImGui::Checkbox("##AutoStepping", &component.AutoStepping);
                ImGui::TableNextColumn();
                ComponentTableReset(component.AutoStepping, true)
            }

            if (component.AutoStepping)
            {
                ImGui::TableNextColumn();
                {
                    ImGui::Text("Stick to floor step down");
                    ImGui::TableNextColumn();
                    ImGui::DragFloat("##StickToFloorStepDown", &component.StickToFloorStepDown.y, -10.0f, 0.01f, 0.0f);
                    ImGui::TableNextColumn();
                    ComponentTableReset(component.StickToFloorStepDown.y, -0.5f);
                }
                ImGui::TableNextColumn();
                {
                    ImGui::Text("Step down extra");
                    ImGui::TableNextColumn();
                    ImGui::DragFloat("##StepDownExtra", &component.StepDownExtra.y, -10.0f, 0.01f, 0.0f);
                    ImGui::TableNextColumn();
                    ComponentTableReset(component.StepDownExtra.y, 0.0f);
                }
                ImGui::TableNextColumn();
                {
                    ImGui::Text("Step up");
                    ImGui::TableNextColumn();
                    ImGui::DragFloat("##StepUp", &component.SteppingStepUp.y, 0.0f, 0.01f, 10.0f);
                    ImGui::TableNextColumn();
                    ComponentTableReset(component.SteppingStepUp.y, 0.4f);
                }
                ImGui::TableNextColumn();
                {
                    ImGui::Text("Step distance");
                    ImGui::TableNextColumn();
                    ImGui::DragFloat("##StepDistance", &component.SteppingForwardDistance, 0.0f, 0.01f, 10.0f);
                    ImGui::TableNextColumn();
                    ComponentTableReset(component.SteppingForwardDistance, 0.250f)
                }
                ImGui::TableNextColumn();
                {
                    ImGui::Text("Step min distance");
                    ImGui::TableNextColumn();
                    ImGui::DragFloat("##StepMinDistance", &component.SteppingMinDistance, 0.0f, 0.01f, component.SteppingForwardDistance);
                    ImGui::TableNextColumn();
                    ComponentTableReset(component.SteppingMinDistance, 0.125f)
                }
            }
        }
        EndComponentTable()
    }
};
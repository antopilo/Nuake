#pragma once
#include "ComponentPanel.h"
#include <src/Scene/Entities/ImGuiHelper.h>
#include <src/Scene/Components/TransformComponent.h>
#include <src/Core/Maths.h>
class TransformPanel : ComponentPanel {

public:
	TransformPanel() {}

	void Draw(Nuake::Entity entity) override
	{
        using namespace Nuake;
        Nuake::TransformComponent& component = entity.GetComponent<Nuake::TransformComponent>();
        ImGui::Indent();
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.f, 0.f));                                                     
        if (ImGui::BeginTable("#transform", 3, ImGuiTableFlags_SizingStretchProp))
        {                                                                               
            ImGui::TableSetupColumn("name", 0, 0.25f);                                  
            ImGui::TableSetupColumn("set", 0, 0.65f);                                   
            ImGui::TableSetupColumn("reset", 0, 0.1f);                                  
                            
            ImGui::TableNextColumn();
            {
                {
                    CompononentPropertyName("Translation");
                    ImGui::TableNextColumn();

                    Vector3 position = component.GetLocalPosition();
                    ImGuiHelper::DrawVec3("Translation##" + entity.GetHandle(), &position);
                    if (position != component.GetLocalPosition())
                        component.SetLocalPosition(position);

                    ImGui::TableNextColumn();

                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1, 1, 1, 0));
                    std::string resetLabel = std::string(ICON_FA_UNDO) + "##ResetTranslation" + std::to_string(entity.GetHandle());
                    if (ImGui::Button(resetLabel.c_str()))
                        component.SetLocalPosition(Vector3(0, 0, 0));

                    ImGui::PopStyleColor();
                }
                ImGui::TableNextColumn();
                {
                    CompononentPropertyName("Rotation");
                    ImGui::TableNextColumn();

                    // Get the current local rotation as a quaternion
                    Quat currentRotation = component.GetLocalRotation();

                    // Convert quaternion to Euler angles in degrees
                    Vector3 eulerDegreesOld = glm::degrees(glm::eulerAngles(currentRotation));

                    // Draw the ImGui widget for rotation
                    ImGuiHelper::DrawVec3("Rotation##" + entity.GetHandle(), &eulerDegreesOld);

                    // Calculate the delta in Euler angles
                    Vector3 eulerDelta = eulerDegreesOld - glm::degrees(glm::eulerAngles(currentRotation));

                    // Apply a small threshold to ignore minor changes
                    if (fabs(eulerDelta.x) < 0.01) eulerDelta.x = 0.0f;
                    if (fabs(eulerDelta.y) < 0.01) eulerDelta.y = 0.0f;
                    if (fabs(eulerDelta.z) < 0.01) eulerDelta.z = 0.0f;

                    // Check if there was a significant rotation change
                    if (glm::length(eulerDelta) > 0.001f)
                    {
                        // Convert delta back to radians
                        Vector3 eulerAnglesDelta = glm::radians(eulerDelta);

                        // Calculate the new rotation quaternion by applying the delta to the current rotation
                        // Apply changes only to the axis that was modified
                        Quat deltaRotation = glm::quat(eulerAnglesDelta);
                        Quat deltaRotationX = glm::angleAxis(eulerAnglesDelta.x, Vector3(1.0f, 0.0f, 0.0f));
                        Quat deltaRotationY = glm::angleAxis(eulerAnglesDelta.y, Vector3(0.0f, 1.0f, 0.0f));
                        Quat deltaRotationZ = glm::angleAxis(eulerAnglesDelta.z, Vector3(0.0f, 0.0f, 1.0f));

                        // Combine the rotations: only the modified axis is affected
                        Quat newRotation = deltaRotationZ * deltaRotationY * deltaRotationX * currentRotation;

                        // Set the new rotation
                        component.SetLocalRotation(newRotation);
                    }
                    ImGui::TableNextColumn();

                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1, 1, 1, 0));
                    std::string resetLabel = std::string(ICON_FA_UNDO) + "##ResetRotation" + std::to_string(entity.GetHandle());
                    if (ImGui::Button(resetLabel.c_str()))
                    {
                        component.SetLocalRotation(Quat(1, 0, 0, 0));
                    }
                    ImGui::PopStyleColor();
                }
                ImGui::TableNextColumn();
                {
                    CompononentPropertyName("Scale");
                    ImGui::TableNextColumn();

                    Vector3 localScale = component.GetLocalScale();
                    ImGuiHelper::DrawVec3("Scale##" + entity.GetHandle(), &localScale);

                    if (localScale != component.GetLocalScale())
                        component.SetLocalScale(localScale);

                    ImGui::TableNextColumn();

                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1, 1, 1, 0));                 
                    std::string resetLabel = std::string(ICON_FA_UNDO) + "##ResetScale" + std::to_string(entity.GetHandle());
                    if (ImGui::Button(resetLabel.c_str()))
                        component.SetLocalScale(Vector3(1, 1, 1));   

                    ImGui::PopStyleColor();
                }
            }
            ImGui::EndTable();
        }
        ImGui::PopStyleVar();
        ImGui::Unindent();
	}
};
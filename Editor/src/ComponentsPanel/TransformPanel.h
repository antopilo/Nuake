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
                    ImGuiHelper::DrawVec3("Translation", &position);
                    if (position != component.GetLocalPosition())
                        component.SetLocalPosition(position);

                    ImGui::TableNextColumn();

                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1, 1, 1, 0));
                    std::string resetLabel = std::string(ICON_FA_UNDO) + "##ResetTranslation";
                    if (ImGui::Button(resetLabel.c_str()))
                        component.SetLocalPosition(Vector3(0, 0, 0));

                    ImGui::PopStyleColor();
                }
                ImGui::TableNextColumn();
                {
                    CompononentPropertyName("Rotation");
                    ImGui::TableNextColumn();

                    Vector3 eulerAngles = glm::eulerAngles(component.GetLocalRotation());
                    Vector3 eulerDegrees = Vector3(glm::degrees(eulerAngles.x), glm::degrees(eulerAngles.y), glm::degrees(eulerAngles.z));
                    ImGuiHelper::DrawVec3("Rotation", &eulerDegrees);
                    Vector3 eulerAnglesDelta = Vector3(glm::radians(eulerDegrees.x), glm::radians(eulerDegrees.y), glm::radians(eulerDegrees.z));

                    float delta = glm::length(eulerAngles - eulerAnglesDelta);
                    if (delta > 0.f)
                    {
                        Quat rotation = QuatFromEuler(eulerAnglesDelta.x, eulerAnglesDelta.y, eulerAnglesDelta.z);
                        //component.SetLocalRotation(rotation);
                    }
                    ImGui::TableNextColumn();

                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1, 1, 1, 0));
                    std::string resetLabel = std::string(ICON_FA_UNDO) + "##ResetRotation";
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
                    ImGuiHelper::DrawVec3("Scale", &localScale);

                    if (localScale != component.GetLocalScale())
                        component.SetLocalScale(localScale);

                    ImGui::TableNextColumn();

                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1, 1, 1, 0));                 
                    std::string resetLabel = std::string(ICON_FA_UNDO) + "##ResetScale";  
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
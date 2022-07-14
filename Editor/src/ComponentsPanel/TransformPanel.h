#pragma once
#include "ComponentPanel.h"
#include <src/Scene/Entities/ImGuiHelper.h>
#include <src/Scene/Components/TransformComponent.h>

class TransformPanel : ComponentPanel {

public:
	TransformPanel() {}

	void Draw(Nuake::Entity entity) override
	{
        using namespace Nuake;
        Nuake::TransformComponent& component = entity.GetComponent<Nuake::TransformComponent>();
        BeginComponentTable(TRANSFORM, Nuake::TransformComponent);
        {
            {
                ImGui::Text("Translation");
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
                ImGui::Text("Rotation");
                ImGui::TableNextColumn();

                Vector3 eulerAngles = glm::eulerAngles(component.GetLocalRotation());
                ImGuiHelper::DrawVec3("Rotation", &eulerAngles);

                Vector3 eulerLocal = glm::eulerAngles(component.GetLocalRotation());
                if (eulerLocal != eulerAngles)
                {
                    Quat rotation = QuatFromEuler(eulerAngles.x, eulerAngles.y, eulerAngles.z);
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
                ImGui::Text("Scale");
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
        EndComponentTable();
	}
};
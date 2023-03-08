#pragma once
#include "ComponentPanel.h"
#include <src/Scene/Components/BoxCollider.h>
#include <src/Core/FileSystem.h>
#include <src/Scene/Entities/ImGuiHelper.h>

class BoxColliderPanel : ComponentPanel {
public:
	BoxColliderPanel() {}

	void Draw(Nuake::Entity entity) override
	{
		if (!entity.HasComponent<Nuake::BoxColliderComponent>())
			return;

		auto& component = entity.GetComponent<Nuake::BoxColliderComponent>();
		BeginComponentTable(BOX COLLIDER, Nuake::BoxColliderComponent);
		{
			{
				ImGui::Text("Size");
				ImGui::TableNextColumn();

				ImGuiHelper::DrawVec3("BoxSize", &component.Size);
				ImGui::TableNextColumn();
				ComponentTableReset(component.Size, glm::vec3(1, 1, 1));
			}
			ImGui::TableNextColumn();
			{
				ImGui::Text("Is Trigger");
				ImGui::TableNextColumn();

				ImGui::Checkbox("##isTrigger", &component.IsTrigger);
				ImGui::TableNextColumn();
				ComponentTableReset(component.IsTrigger, false);
			}
		}
		EndComponentTable();
	}
};
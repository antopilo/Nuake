#pragma once
#include "ComponentPanel.h"
#include <src/Scene/Components/SphereCollider.h>
#include <src/Core/FileSystem.h>
#include <src/Scene/Entities/ImGuiHelper.h>

class SphereColliderPanel : ComponentPanel {
public:
	SphereColliderPanel() {}

	void Draw(Nuake::Entity entity) override
	{
		if (!entity.HasComponent<Nuake::SphereColliderComponent>())
			return;

		auto& component = entity.GetComponent<Nuake::SphereColliderComponent>();
		BeginComponentTable(SPHERE COLLIDER, Nuake::SphereColliderComponent);
		{
			{
				ImGui::Text("Size");
				ImGui::TableNextColumn();
				ImGui::DragFloat("##Radius", &component.Radius);
				ImGui::TableNextColumn();
				ComponentTableReset(component.Radius, 0.5f);
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
#pragma once
#include "ComponentPanel.h"
#include <src/Scene/Components/CapsuleColliderComponent.h>

class CapsuleColliderPanel: ComponentPanel
{
public:
	CapsuleColliderPanel() = default;

	void Draw(Nuake::Entity entity) override
	{
		if (!entity.HasComponent<Nuake::CapsuleColliderComponent>())
			return;

		auto& component = entity.GetComponent<Nuake::CapsuleColliderComponent>();
		BeginComponentTable(CAPSULE COLLIDER, Nuake::CapsuleColliderComponent);
		{
			{
				ImGui::Text("Radius");
				ImGui::TableNextColumn();
				ImGui::DragFloat("##Radius", &component.Radius);
				ImGui::TableNextColumn();
				ComponentTableReset(component.Radius, 0.5f)
			}
			ImGui::TableNextColumn();
			{
				ImGui::Text("Height");
				ImGui::TableNextColumn();

				ImGui::DragFloat("##Height", &component.Height);
				ImGui::TableNextColumn();
				ComponentTableReset(component.Height, 1.0f)
			}
		}
		EndComponentTable()
	}
};
#pragma once
#include "ComponentPanel.h"

#include <src/Scene/Components/CylinderColliderComponent.h>

class CylinderColliderPanel : ComponentPanel
{
public:
	CylinderColliderPanel() = default;

	void Draw(Nuake::Entity entity) override
	{
		using namespace Nuake;

		if (!entity.HasComponent<CylinderColliderComponent>())
		{
			return;
		}

		auto& [Cylinder, Radius, Height, IsTrigger] = entity.GetComponent<CylinderColliderComponent>();
		BeginComponentTable(CYLINDER COLLIDER, CylinderColliderComponent)
		{
			{
				ImGui::Text("Radius");
				ImGui::TableNextColumn();
				ImGui::DragFloat("##Radius", &Radius, 0.01f, 0.001f);
				Radius = std::max(Radius, 0.001f);
				ImGui::TableNextColumn();
				ComponentTableReset(Radius, 0.5f)
			}
			ImGui::TableNextColumn();
			{
				ImGui::Text("Height");
				ImGui::TableNextColumn();
				ImGui::DragFloat("##Height", &Height, 0.01f, 0.0001f);
				Height = std::max(Height, 0.001f);
				ImGui::TableNextColumn();
				ComponentTableReset(Height, 1.0f)
			}
		}
		EndComponentTable()
	}
};
#pragma once
#include "ComponentPanel.h"

#include <src/Scene/Components/CapsuleColliderComponent.h>

class CapsuleColliderPanel : ComponentPanel
{
public:
	CapsuleColliderPanel() = default;

	void Draw(Nuake::Entity entity) override
	{
		using namespace Nuake;

		if (!entity.HasComponent<CapsuleColliderComponent>())
		{
			return;
		}

		auto& [Capsule, Radius, Height, IsTrigger] = entity.GetComponent<CapsuleColliderComponent>();
		BeginComponentTable(CAPSULE COLLIDER, CapsuleColliderComponent)
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
				ImGui::DragFloat("##Height", &Height, 0.01f, 0.001f);
				Height = std::max(Height, 0.001f);
				ImGui::TableNextColumn();
				ComponentTableReset(Height, 1.0f)
			}
			ImGui::TableNextColumn();
			{
				ImGui::Text("Is Trigger");
				ImGui::TableNextColumn();

				ImGui::Checkbox("##isTrigger", &IsTrigger);
				ImGui::TableNextColumn();
				ComponentTableReset(IsTrigger, false);
			}
		}
		EndComponentTable()
	}
};
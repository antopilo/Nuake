#pragma once
#include "ComponentPanel.h"

#include <src/Core/FileSystem.h>
#include <src/Core/Maths.h>
#include <src/Scene/Components/BoxCollider.h>
#include <src/Scene/Entities/ImGuiHelper.h>

class BoxColliderPanel : ComponentPanel 
{
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

				ImGuiHelper::DrawVec3("BoxSize", &component.Size, 0.5f, 100.0, 0.01f);
				component.Size = glm::abs(component.Size);

				component.Size.x = std::max(component.Size.x, 0.0001f);
				component.Size.y = std::max(component.Size.y, 0.0001f);
				component.Size.z = std::max(component.Size.z, 0.0001f);

				ImGui::TableNextColumn();
				ComponentTableReset(component.Size, Nuake::Vector3(0.5f, 0.5f, 0.5f));
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
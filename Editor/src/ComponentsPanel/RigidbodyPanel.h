#pragma once
#include "ComponentPanel.h"
#include <src/Scene/Components/RigidbodyComponent.h>
#include <src/Core/FileSystem.h>

class RigidbodyPanel : ComponentPanel {

public:
	RigidbodyPanel() {}

	void Draw(Nuake::Entity entity) override
	{
		if (!entity.HasComponent<Nuake::RigidBodyComponent>())
			return;

		auto& component = entity.GetComponent<Nuake::RigidBodyComponent>();
		BeginComponentTable(RIGIDBODY, Nuake::RigidBodyComponent);
		{
			{
				ImGui::Text("Mass");
				ImGui::TableNextColumn();

				ImGui::DragFloat("##Mass", &component.Mass, 0.01f, 0.1f);
				component.Mass = std::max(component.Mass, 0.0f);
				ImGui::TableNextColumn();
				ComponentTableReset(component.Mass, 0.0f);
			}
			ImGui::TableNextColumn();
			{
				ImGui::Text("Lock X axis");
				ImGui::TableNextColumn();

				ImGui::Checkbox("##lockx", &component.LockX);
				
				ImGui::TableNextColumn();
				ComponentTableReset(component.LockX, false);
			}
			ImGui::TableNextColumn();
			{
				ImGui::Text("Lock Y axis");
				ImGui::TableNextColumn();

				ImGui::Checkbox("##locky", &component.LockY);

				ImGui::TableNextColumn();
				ComponentTableReset(component.LockY, false);
			}
			ImGui::TableNextColumn();
			{
				ImGui::Text("Lock Z axis");
				ImGui::TableNextColumn();

				ImGui::Checkbox("##lockz", &component.LockZ);

				ImGui::TableNextColumn();
				ComponentTableReset(component.LockZ, false);
			}
			ImGui::TableNextColumn();
		}
		EndComponentTable();
	}
};
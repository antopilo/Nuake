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

				ImGui::DragFloat("##Mass", &component.Mass, 0.1f, 0.1f);
				ImGui::TableNextColumn();
				ComponentTableReset(component.Mass, 0.0f);
			}
		}
		EndComponentTable();
	}
};
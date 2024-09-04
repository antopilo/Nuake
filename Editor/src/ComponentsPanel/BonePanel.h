#pragma once
#include "ComponentPanel.h"

#include "src/FileSystem/FileSystem.h"
#include <src/Core/Maths.h>
#include <src/Scene/Components/BoneComponent.h>
#include <src/Scene/Entities/ImGuiHelper.h>

class BonePanel : ComponentPanel
{
public:
	BonePanel() {}

	void Draw(Nuake::Entity entity) override
	{
		using namespace Nuake;

		if (!entity.HasComponent<BoneComponent>())
			return;

		auto& component = entity.GetComponent<BoneComponent>();
		BeginComponentTable(BONE, BoneComponent);
		{
			{
				ImGui::Text("Name");
				ImGui::TableNextColumn();

				ImGui::InputText("##BoneName", &component.Name);
				ImGui::TableNextColumn();

				ComponentTableReset(component.Name, "");
			}
		}
		EndComponentTable();
	}
};
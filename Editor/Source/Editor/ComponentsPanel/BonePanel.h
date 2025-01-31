#pragma once
#include "ComponentPanel.h"

#include "Nuake/FileSystem/FileSystem.h"
#include <Nuake/Core/Maths.h>
#include <Nuake/Scene/Components/BoneComponent.h>
#include <Nuake/Scene/Entities/ImGuiHelper.h>

class BonePanel
{
public:
	static void Draw(Nuake::Entity& entity, entt::meta_any& componentInstance)
	{
		using namespace Nuake;
        
		Nuake::BoneComponent* componentPtr = componentInstance.try_cast<Nuake::BoneComponent>();
		if (componentPtr == nullptr)
		{
			return;
		}
		Nuake::BoneComponent& component = *componentPtr;
        
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
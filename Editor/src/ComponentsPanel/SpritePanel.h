#pragma once
#include "ComponentPanel.h"

#include <src/Core/FileSystem.h>
#include <src/Core/Maths.h>
#include <src/Scene/Components/SpriteComponent.h>
#include <src/Scene/Entities/ImGuiHelper.h>


class SpritePanel : ComponentPanel
{
public:
	SpritePanel() = default;
	~SpritePanel() = default;

	void Draw(Nuake::Entity entity) override
	{
		if (!entity.HasComponent<Nuake::SpriteComponent>())
		{
			return;
		}

		auto& component = entity.GetComponent<Nuake::SpriteComponent>();
		BeginComponentTable(SPRITE, Nuake::SpriteComponent);
		{
			{
				ImGui::Text("Sprite");
				ImGui::TableNextColumn();

				std::string path = component.SpritePath;
				ImGui::Button(path.empty() ? "Drag image" : component.SpritePath.c_str(), ImVec2(ImGui::GetContentRegionAvail().x, 0));
				if (ImGui::BeginDragDropTarget())
				{
					if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("_Image"))
					{
						char* file = (char*)payload->Data;

						std::string fullPath = std::string(file, 512);
						path = Nuake::FileSystem::AbsoluteToRelative(std::move(fullPath));
						component.SpritePath = path;

						component.LoadSprite();
					}
					ImGui::EndDragDropTarget();
				}

				ImGui::TableNextColumn();
				ComponentTableReset(component.LockYRotation, false);
			}
			ImGui::TableNextColumn();
			{
				ImGui::Text("Billboard");
				ImGui::TableNextColumn();

				ImGui::Checkbox("##billboard", &component.Billboard);
				ImGui::TableNextColumn();

				ComponentTableReset(component.Billboard, false);
			}
			ImGui::TableNextColumn();
			{
				ImGui::Text("Lock Y rotation");
				ImGui::TableNextColumn();

				ImGui::Checkbox("##lockYRotation", &component.LockYRotation);
				ImGui::TableNextColumn();

				ComponentTableReset(component.LockYRotation, false);
			}
			ImGui::TableNextColumn();
			{
				ImGui::Text("Position Based");
				if (ImGui::BeginItemTooltip())
				{
					ImGui::Text("Orientation is based on the position of the camera or the orientation of the camera.");
					ImGui::EndTooltip();
				}
				
				ImGui::TableNextColumn();

				ImGui::Checkbox("##positionbased", &component.PositionFacing);
				ImGui::TableNextColumn();

				ComponentTableReset(component.LockYRotation, false);
			}
		}
		EndComponentTable();
	}
};
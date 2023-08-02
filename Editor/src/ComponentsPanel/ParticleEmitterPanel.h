#pragma once
#include "ComponentPanel.h"

#include <src/Core/FileSystem.h>
#include <src/Core/Maths.h>
#include <src/Scene/Components/ParticleEmitterComponent.h>
#include <src/Scene/Entities/ImGuiHelper.h>

class ParticleEmitterPanel : ComponentPanel
{
public:
	ParticleEmitterPanel() {}

	void Draw(Nuake::Entity entity) override
	{
		if (!entity.HasComponent<Nuake::ParticleEmitterComponent>())
			return;

		auto& component = entity.GetComponent<Nuake::ParticleEmitterComponent>();
		BeginComponentTable(PARTICLE EMITTER, Nuake::ParticleEmitterComponent);
		{
			{
				ImGui::Text("Particle Color");
				ImGui::TableNextColumn();

				ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
				ImGui::ColorEdit3("##lightcolor", &component.ParticleColor.r, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel);
				ImGui::PopItemWidth();

				ImGui::TableNextColumn();
				ComponentTableReset(component.ParticleColor, Nuake::Vector4(1, 1, 1, 1));
			}
			ImGui::TableNextColumn();
			{
				ImGui::Text("Amount");
				ImGui::TableNextColumn();

				ImGui::DragFloat("##ParticleAmount", &component.Amount, 0.1f, 0.0f, 500.0f);
				ImGui::TableNextColumn();
				ComponentTableReset(component.Amount, 10.0f);
			}
			ImGui::TableNextColumn();
			{
				ImGui::Text("Rate");
				ImGui::TableNextColumn();

				ImGui::DragFloat("##ParticleRate", &component.Rate, 0.1f, 0.0f, 10.0f);
				ImGui::TableNextColumn();
				ComponentTableReset(component.Rate, 0.0f);
			}
			ImGui::TableNextColumn();
			{
				ImGui::Text("Life");
				ImGui::TableNextColumn();

				ImGui::DragFloat("##ParticleLife", &component.Life, 0.1f, 0.0f, 100.0f);
				ImGui::TableNextColumn();
				ComponentTableReset(component.Life, 5.0f);
			}
			ImGui::TableNextColumn();
			{
				ImGui::Text("Gravity");
				ImGui::TableNextColumn();

				ImGuiHelper::DrawVec3("Gravity", &component.Gravity);
				ImGui::TableNextColumn();
				ComponentTableReset(component.Gravity, Nuake::Vector3(0, -1, 0));
			}
			ImGui::TableNextColumn();
			{
				ImGui::Text("Gravity Random");
				ImGui::TableNextColumn();
				ImGui::DragFloat("##GravityRandom", &component.GravityRandom, 0.01f, 0.0f, 1.0f);
				ImGui::TableNextColumn();
				ComponentTableReset(component.GravityRandom, 0.0f);
			}
			ImGui::TableNextColumn();
			{
				ImGui::Text("Radius");
				ImGui::TableNextColumn();
				ImGui::DragFloat("##ParticleRadius", &component.Radius, 0.01f, 0.0f, 10.0f);
				ImGui::TableNextColumn();
				ComponentTableReset(component.Radius, 1.0f);
			}
		}
		EndComponentTable();
	}
};
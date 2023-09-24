#pragma once
#include "ComponentPanel.h"

#include <src/Core/FileSystem.h>
#include <src/Core/Maths.h>
#include <src/Scene/Components/ParticleEmitterComponent.h>
#include <src/Scene/Entities/ImGuiHelper.h>
#include "MaterialEditor.h"
#include <src/Resource/ResourceLoader.h>
#include <src/Resource/ResourceLoader.h>

class ParticleEmitterPanel : ComponentPanel
{
public:
	Scope<ModelResourceInspector> _modelInspector;

	ParticleEmitterPanel() {}

	void Draw(Nuake::Entity entity) override
	{
		if (!entity.HasComponent<Nuake::ParticleEmitterComponent>())
			return;

		auto& component = entity.GetComponent<Nuake::ParticleEmitterComponent>();
		BeginComponentTable(PARTICLE EMITTER, Nuake::ParticleEmitterComponent);
		{
		
			{
				ImGui::Text("Particle Material");
				ImGui::TableNextColumn();

				std::string label = "Empty";
				if (!component.ParticleMaterial->Path.empty())
				{
					label = component.ParticleMaterial->Path;
				}

				if (ImGui::Button(label.c_str(), ImVec2(ImGui::GetContentRegionAvail().x, 0)))
				{
					
				}

				if (ImGui::BeginDragDropTarget())
				{
					if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("_Material"))
					{
						char* file = (char*)payload->Data;
						std::string fullPath = std::string(file, 256);

						fullPath = Nuake::FileSystem::AbsoluteToRelative(fullPath);

						Ref<Nuake::Material> material = Nuake::ResourceLoader::LoadMaterial(fullPath);
						component.ParticleMaterial = material;
					}
					ImGui::EndDragDropTarget();
				}

				//std::string childId = "materialEditorParticle";
				//ImGui::BeginChild(childId.c_str(), ImVec2(0, 0), false);
				//{
				//	MaterialEditor editor;
				//	editor.Draw(component.ParticleMaterial);
				//}
				//ImGui::EndChild();

				ImGui::TableNextColumn();
				//ComponentTableReset(component.ParticleColor, Nuake::Vector4(1, 1, 1, 1));
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
				ImGui::Text("Particle Scale");
				ImGui::TableNextColumn();

				ImGuiHelper::DrawVec3("##particleSize", &component.ParticleScale);

				ImGui::TableNextColumn();
				ComponentTableReset(component.ParticleScale, Nuake::Vector3(0.1, 0.1, 0.1));
			}
			ImGui::TableNextColumn();
			{
				ImGui::Text("Particle Scale Random");
				ImGui::TableNextColumn();

				ImGui::DragFloat("##particleSizeRandom", &component.ScaleRandomness, 0.01f, 0.0f);

				ImGui::TableNextColumn();
				ComponentTableReset(component.ScaleRandomness, 0.0f);
			}
			ImGui::TableNextColumn();
			{
				ImGui::Text("Global Space");
				ImGui::TableNextColumn();

				ImGui::Checkbox("##globalSpace", &component.GlobalSpace);

				ImGui::TableNextColumn();
				ComponentTableReset(component.GlobalSpace, false);
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
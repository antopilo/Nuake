#include "ModelResourceInspector.h"
#include <imgui/imgui.h>
#include "MaterialEditor.h"

ModelResourceInspector::ModelResourceInspector(Ref<Nuake::Model> model)
{
	_model = model;
}

void ModelResourceInspector::Draw()
{
	ImGui::BeginChild("modelInspector", ImVec2(0, 100 * std::size(_model->GetMeshes())), true);
	{
		for (uint32_t i = 0; i < std::size(_model->GetMeshes()); i++)
		{
			std::string headerLabel = "Mesh " + std::to_string(i);
			if (ImGui::CollapsingHeader(headerLabel.c_str()))
			{
				std::string childId = "materialEditor" + std::to_string(i);
				ImGui::BeginChild(childId.c_str(), ImVec2(0, 0), false);
				{
					MaterialEditor editor;
					editor.Draw(_model->GetMeshes().at(i)->GetMaterial());
				}
				ImGui::EndChild();
			}
		}
	}

	ImGui::EndChild();
}
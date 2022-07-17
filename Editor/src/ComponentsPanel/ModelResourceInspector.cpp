#include "ModelResourceInspector.h"
#include <imgui/imgui.h>

ModelResourceInspector::ModelResourceInspector(Ref<Nuake::Model> model)
{
	_model = model;
}

void ModelResourceInspector::Draw()
{
	ImGui::BeginChild("modelInspector",ImVec2(0,0), true);
	{
		for (uint32_t i = 0; i < std::size(_model->GetMeshes()); i++)
		{
			std::string headerLabel = "Mesh " + std::to_string(i);
			if (ImGui::CollapsingHeader(headerLabel.c_str()))
			{

			}
		}
	}
	ImGui::EndChild();
}
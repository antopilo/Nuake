#include "Viewport.h"
#include <src/Vendors/imgui/imgui.h>

Viewport::Viewport(const std::string& name, Ref<Nuake::Texture> texture)
{
	Name = name;
	Texture = texture;
}

void Viewport::Draw()
{
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));

	if (ImGui::Begin(Name.c_str()))
	{
		ImGui::Image((void*)Texture->GetID(), ImGui::GetContentRegionAvail(), ImVec2(0, 1), ImVec2(1, 0));
	}
	ImGui::PopStyleVar();

	ImGui::End();
}
#include "ImGuiTextHelper.h"

#include <imgui/imgui.h>

void ImGuiTextSTD(const std::string& label, std::string& value)
{
	char buffer[256];
	memset(buffer, 0, sizeof(buffer));
	strncpy_s(buffer, value.c_str(), sizeof(buffer));
	if (ImGui::InputText(label.c_str(), buffer, sizeof(buffer)))
	{

		value = std::string(buffer);
	}
}

void ImGuiTextMultiline(const std::string& label, std::string& value)
{
	char buffer[256];
	memset(buffer, 0, sizeof(buffer));
	strncpy_s(buffer, value.c_str(), sizeof(buffer));
	if (ImGui::InputTextMultiline(label.c_str(), buffer, sizeof(buffer)))
	{
		value = std::string(buffer);
	}
}
#include "PopupHelper.h"
#include "imgui/imgui.h"


void PopupHelper::Confirmation(const std::string& id)
{
    ImGui::TextWrapped(id.c_str());

    ImGui::OpenPopup(id.c_str());
}

bool PopupHelper::DefineDialog(const std::string& id, const std::string& text)
{
    bool result = false;

    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

    if (ImGui::BeginPopupModal(id.c_str(), NULL, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::Text((text + "\n\n").c_str());
        ImGui::Separator();

        if (ImGui::Button("OK", ImVec2(120, 0))) { result = true; ImGui::CloseCurrentPopup(); }
        ImGui::SetItemDefaultFocus();
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(120, 0))) { ImGui::CloseCurrentPopup(); }
        ImGui::EndPopup();
    }

    return result;
}

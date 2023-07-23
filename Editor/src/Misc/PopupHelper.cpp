#include "PopupHelper.h"
#include "imgui/imgui.h"
#include "regex"

void PopupHelper::OpenPopup(const std::string& id)
{
    ImGui::TextWrapped(id.c_str());

    ImGui::OpenPopup(id.c_str());
}

bool PopupHelper::DefineConfirmationDialog(const std::string& id, const std::string& text)
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

bool PopupHelper::DefineTextDialog(const std::string& id, std::string& currentText)
{
    bool result = false;

    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

    if (ImGui::BeginPopupModal(id.c_str(), NULL, ImGuiWindowFlags_AlwaysAutoResize))
    {
        bool isSanitized = true;

        char buffer[256];
        memset(buffer, 0, sizeof(buffer));
        std::strncpy(buffer, currentText.c_str(), sizeof(buffer));
        if (ImGui::InputText("", buffer, sizeof(buffer)))
        {
            currentText = std::string(buffer);
        }

        // returns false when there is a match, quirky
        if (!regex_match(buffer, std::regex("[^ \\ / :*? \"<>|]+")))
        {
            isSanitized = false;
        }

        if (currentText.empty())
        {
            ImGui::TextColored(ImVec4(0.76, 0.45, 0.47, 1.0), "A file name can't be empty.");
        } 
        else if (!isSanitized)
        {
            ImGui::TextColored(ImVec4(0.76, 0.45, 0.47, 1.0), "A file name can't contain any of the following characters: \\/:*?\"<>|");
        }

        ImGui::Separator();

        if (ImGui::Button("OK", ImVec2(120, 0))) 
        { 
            if (isSanitized)
            {
                result = true;
                ImGui::CloseCurrentPopup();
            }
        }
        ImGui::SetItemDefaultFocus();
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(120, 0))) { ImGui::CloseCurrentPopup(); }
        ImGui::EndPopup();
    }

    return result;
}

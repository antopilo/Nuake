#include "LoggerWidget.h"

#include "src/Core/Logger.h"

#include "src/UI/ImUI.h"

#include "src/Resource/Project.h"
#include "Engine.h"

using namespace Nuake;

void LoggerWidget::Update(float ts)
{

}

void LoggerWidget::Draw()
{
    if (BeginWidgetWindow("Logger"))
    {
        if (ImGui::Button("Clear", ImVec2(60, 28)))
        {
            Logger::ClearLogs();
            //SetStatusMessage("Logs cleared.");
        }

        ImGui::SameLine();

        if (ImGui::Button(ICON_FA_FILTER, ImVec2(30, 28)))
        {
            ImGui::OpenPopup("filter_popup");
        }

        ImGui::SameLine();

        bool isEnabled = LogErrors;
        if (ImGui::BeginPopup("filter_popup"))
        {
            ImGui::SeparatorText("Filters");
            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(2, 2));
            ImGui::PushStyleColor(ImGuiCol_Button, IM_COL32(0, 0, 0, 0));
            ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 100);

            if (isEnabled)
            {
                Color color = Engine::GetProject()->Settings.PrimaryColor;
                ImGui::PushStyleColor(ImGuiCol_Button, { color.r, color.g, color.b, 1.0f });
            }

            if (ImGui::Button((std::string(ICON_FA_BAN) + " Error").c_str()))
            {
                LogErrors = !LogErrors;
            }

            UI::Tooltip("Display Errors");
            if (isEnabled)
            {
                ImGui::PopStyleColor();
            }

            isEnabled = LogWarnings;
            if (isEnabled)
            {
                Color color = Engine::GetProject()->Settings.PrimaryColor;
                ImGui::PushStyleColor(ImGuiCol_Button, { color.r, color.g, color.b, 1.0f });
            }

            if (ImGui::Button((std::string(ICON_FA_EXCLAMATION_TRIANGLE) + " Warning").c_str()))
            {
                LogWarnings = !LogWarnings;
            }

            UI::Tooltip("Display Warnings");
            if (isEnabled)
            {
                ImGui::PopStyleColor();
            }

            isEnabled = LogDebug;
            if (isEnabled)
            {
                Color color = Engine::GetProject()->Settings.PrimaryColor;
                ImGui::PushStyleColor(ImGuiCol_Button, { color.r, color.g, color.b, 1.0f });
            }

            if (ImGui::Button((std::string(ICON_FA_INFO) + " Info").c_str()))
            {
                LogDebug = !LogDebug;
            }

            UI::Tooltip("Display Verbose");
            if (isEnabled)
            {
                ImGui::PopStyleColor();
            }

            ImGui::PopStyleColor();
            ImGui::PopStyleVar(2);

            ImGui::EndPopup();
        }

        ImGui::SameLine();

        isEnabled = AutoScroll;
        if (isEnabled)
        {
            Color color = Engine::GetProject()->Settings.PrimaryColor;
            ImGui::PushStyleColor(ImGuiCol_Button, { color.r, color.g, color.b, 1.0f });
        }

        if (ImGui::Button(ICON_FA_ARROW_DOWN, ImVec2(30, 28)))
        {
            AutoScroll = !AutoScroll;
        }

        UI::Tooltip("Auto-Scroll");
        if (isEnabled)
        {
            ImGui::PopStyleColor();
        }

        //ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
        //if (ImGui::BeginChild("Log window", ImGui::GetContentRegionAvail(), false))
        //{
            //ImGui::PopStyleVar();
        ImGuiTableFlags flags = ImGuiTableFlags_ScrollY | ImGuiTableFlags_RowBg | ImGuiTableFlags_Hideable;
        if (ImGui::BeginTable("LogTable", 3, flags))
        {
            ImGui::TableSetupColumn("Severity", ImGuiTableColumnFlags_WidthFixed, 64.0f);
            ImGui::TableSetupColumn("Time", ImGuiTableColumnFlags_WidthFixed, 64.0f);
            ImGui::TableSetupColumn("Message", ImGuiTableColumnFlags_WidthStretch, 1.0f);
            ImGui::TableNextColumn();
            ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(4, 4));

            for (auto& l : Logger::GetLogs())
            {
                if (l.type == LOG_TYPE::VERBOSE && !LogDebug)
                    continue;
                if (l.type == LOG_TYPE::WARNING && !LogWarnings)
                    continue;
                if (l.type == LOG_TYPE::CRITICAL && !LogErrors)
                    continue;

                std::string severityText = "";
                if (l.type == LOG_TYPE::VERBOSE)
                    severityText = "verbose";
                else if (l.type == LOG_TYPE::WARNING)
                    severityText = "warning";
                else
                    severityText = "critical";

                ImVec4 redColor = ImVec4(0.6, 0.1f, 0.1f, 0.2f);
                ImVec4 yellowColor = ImVec4(0.6, 0.6f, 0.1f, 0.2f);
                ImVec4 colorGreen = ImVec4(0.59, 0.76, 0.47, 1.0);
                ImGui::PushStyleColor(ImGuiCol_Text, colorGreen);
                ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, ImGui::GetColorU32(ImVec4(0.59, 0.76, 0.47, 0.2)), -1);
                const std::string timeString = " [" + l.time + "]";
                ImGui::Text(timeString.c_str());
                ImGui::PopStyleColor();

                ImGui::TableNextColumn();

                ImVec4 colorBlue = ImVec4(98 / 255.0, 174 / 255.0, 239 / 255.0, 1.);
                ImGui::PushStyleColor(ImGuiCol_Text, colorBlue);
                ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, ImGui::GetColorU32(ImVec4(98 / 255.0, 174 / 255.0, 239 / 255.0, 0.2)), -1);
                ImGui::Text(l.logger.c_str());
                ImGui::PopStyleColor();

                ImGui::TableNextColumn();

                ImVec4 color = ImVec4(1, 1, 1, 1.0);
                ImGui::PushStyleColor(ImGuiCol_Text, color);

                if (l.type == CRITICAL)
                {
                    ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, ImGui::GetColorU32(redColor), -1);
                }
                else if (l.type == WARNING)
                {
                    ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, ImGui::GetColorU32(yellowColor), -1);
                }
                else
                {
                    ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, ImGui::GetColorU32(ImVec4(1, 1, 1, 0.0)), -1);
                }

                std::string displayMessage = l.message;
                if (l.count > 0)
                {
                    displayMessage += "(" + std::to_string(l.count) + ")";
                }

                ImGui::TextWrapped(displayMessage.c_str());
                ImGui::PopStyleColor();

                ImGui::TableNextColumn();
            }
            ImGui::PopStyleVar();

            if (AutoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
            {
                ImGui::SetScrollHereY(1.0f);
            }

            ImGui::EndTable();
        }
    }
    ImGui::End();
}
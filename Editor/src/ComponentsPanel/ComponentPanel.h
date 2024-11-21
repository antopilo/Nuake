#pragma once
#include "src/Scene/Entities/Entity.h"

#include "../Misc/InterfaceFonts.h"

#include "src/UI/ImUI.h"
#include <src/Vendors/imgui/imgui.h>
#include <src/Resource/FontAwesome5.h>


#define MenuItemComponent(label, Component)     \
        if(entity.HasComponent<Component>())    \
            ImGui::Text(label);                 \
        else if (ImGui::MenuItem(label))        \
            entity.AddComponent<Component>();

#define CompononentPropertyName(name) \
ImGui::AlignTextToFramePadding(); \
ImGui::Text(##name);

#define BeginComponentTable(name, component)                                            \
    UIFont* boldFont = new UIFont(Fonts::Bold);                                         \
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.f, 0.f));                   \
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.f, 8.f));                  \
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 0.0f);                             \
    bool removed = false;                                                               \
    bool headerOpened = ImGui::CollapsingHeader(#name, ImGuiTreeNodeFlags_DefaultOpen); \
    ImGui::PopStyleVar();                                                               \
     if (strcmp(#name, "TRANSFORM") != 0 && ImGui::BeginPopupContextItem())             \
     {                                                                                  \
        if (ImGui::Selectable("Remove")) { removed = true; }                            \
        ImGui::EndPopup();                                                              \
     }                                                                                  \
                                                                                        \
    if(removed)                                                                         \
    {                                                                                   \
        entity.RemoveComponent<component>();                                            \
        ImGui::PopStyleVar();                                                           \
        delete boldFont;                                                                \
    }                                                                                   \
    else if (headerOpened)                                                              \
    {                                                                                   \
        delete boldFont;                                                                \
        ImGui::PopStyleVar();                                                           \
        ImGui::Indent();                                                                \
        if (ImGui::BeginTable(#name, 3, ImGuiTableFlags_SizingStretchProp))             \
        {                                                                               \
            ImGui::TableSetupColumn("name", 0, 0.25f);                                  \
            ImGui::TableSetupColumn("set", 0, 0.65f);                                   \
            ImGui::TableSetupColumn("reset", 0, 0.1f);                                  \
                                                                                        \
            ImGui::TableNextColumn();


#define EndComponentTable()             \
            ImGui::EndTable();          \
        }                               \
        ImGui::Unindent();              \
    }                                   \
    else                                \
    {                                   \
        ImGui::PopStyleVar();           \
        delete boldFont;                \
    }                                   \
    ImGui::PopStyleVar(); \
   

#define ComponentTableReset(setting, value)                                     \
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1, 1, 1, 0));                 \
    std::string resetLabel = std::string(ICON_FA_UNDO) + "##Reset" + #setting;  \
    if (ImGui::Button(resetLabel.c_str())) setting = value;                     \
        ImGui::PopStyleColor(); 


#define ComponentDropDown(arrayData, enumData, value)           \
    const char* current_item = arrayData[(int)value];           \
    if (ImGui::BeginCombo("Type", current_item))                \
    {                                                           \
        for (int n = 0; n < IM_ARRAYSIZE(arrayData); n++)       \
        {                                                       \
            bool is_selected = (current_item == arrayData[n]);  \
            if (ImGui::Selectable(arrayData[n], is_selected))   \
            {                                                   \
                current_item = arrayData[n];                    \
                value = (enumData)n;                            \
            }                                                   \
            if (is_selected)                                    \
                ImGui::SetItemDefaultFocus();                   \
        }                                                       \
        ImGui::EndCombo();                                      \
    }                                                           


class ComponentPanel {
public:
	virtual void Draw(Nuake::Entity entity);
};

inline void ComponentPanel::Draw(Nuake::Entity entity) {}

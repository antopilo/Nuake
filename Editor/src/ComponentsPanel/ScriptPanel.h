#pragma once
#include "ComponentPanel.h"
#include <src/Scene/Components/WrenScriptComponent.h>
#include <src/Core/FileSystem.h>

class ScriptPanel : ComponentPanel {

public:
    ScriptPanel() {}

    void Draw(Nuake::Entity entity) override
    {
        if (!entity.HasComponent<Nuake::WrenScriptComponent>())
            return;

        Nuake::WrenScriptComponent& component = entity.GetComponent<Nuake::WrenScriptComponent>();
        BeginComponentTable(SCRIPT, Nuake::WrenScriptComponent);
        {
            {
                ImGui::Text("Script");
                ImGui::TableNextColumn();

                std::string path = component.Script;
                ImGui::Button(component.Script.c_str(), ImVec2(ImGui::GetContentRegionAvail().x, 0));
                if (ImGui::BeginDragDropTarget())
                {
                    if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("_Script"))
                    {
                        char* file = (char*)payload->Data;

                        std::string fullPath = std::string(file, 512);
                        path = Nuake::FileSystem::AbsoluteToRelative(std::move(fullPath));

                        component.LoadScript(path);
                    }
                    ImGui::EndDragDropTarget();
                }

                component.Script = path;
                ImGui::TableNextColumn();

                ComponentTableReset(component.Script, "");
            }
            ImGui::TableNextColumn();
            {
                ImGui::Text("Module");
                ImGui::TableNextColumn();

                // Here we create a dropdown for every modules
                auto& wrenScript = component.mWrenScript;
                if (wrenScript)
                {
                    auto modules = wrenScript->GetModules();

                    std::vector<const char*> modulesC;

                    for (auto& m : modules)
                    {
                        modulesC.push_back(m.c_str());
                    }
                    static int currentModule = (int)component.mModule;
                    ImGui::Combo("##WrenModule", &currentModule, &modulesC[0], modules.size());
                    component.mModule = currentModule;
                }
                
                ImGui::TableNextColumn();
                //ComponentTableReset(component.Class, "");
            }
        }
        EndComponentTable();
    }
};
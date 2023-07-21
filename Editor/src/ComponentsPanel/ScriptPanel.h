#pragma once
#include "ComponentPanel.h"
#include <src/Scene/Components/WrenScriptComponent.h>
#include <src/Core/FileSystem.h>

const std::string TEMPLATE_SCRIPT_FIRST = R"(import "Nuake:Engine" for Engine 
import "Nuake:ScriptableEntity" for ScriptableEntity 
import "Nuake:Input" for Input 
import "Nuake:Math" for Vector3, Math 
import "Nuake:Scene" for Scene 

class )";

const std::string TEMPLATE_SCRIPT_SECOND = R"( is ScriptableEntity { 
        construct new() { 
        } 

        // Called when the scene gets initialized 
        init() { 
            // Engine.Log("Hello World!") 
        } 
 
        // Called every update 
        update(ts) { 
        } 
 
        // Called 90 times per second 
        fixedUpdate(ts) { 
        }  
         
        // Called on shutdown 
        exit() { 
        } 
} 
)";


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
                ImGui::Button( path.empty() ? "Create New" : component.Script.c_str(), ImVec2(ImGui::GetContentRegionAvail().x, 0));
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

                // Double click on file
                if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0))
                {
                    if(!component.Script.empty())
                    {
                        Nuake::OS::OpenIn(component.mWrenScript->GetFile()->GetAbsolutePath());
                    }
                    else
                    {
                        // TODO: Turn into command (Undo/Redo)
                        
                        std::string pathCreation = Nuake::FileDialog::SaveFile("*.wren");
                        
                        if (!pathCreation.empty())
                        {
                            if (!Nuake::String::EndsWith(pathCreation, ".wren"))
                            {
                                pathCreation += ".wren";
                            }
                            
                            std::string fileName = Nuake::String::ToUpper(Nuake::FileSystem::GetFileNameFromPath(pathCreation));
                            fileName = Nuake::String::RemoveWhiteSpace(fileName);
			            
                            if(!Nuake::String::IsDigit(fileName[0]))
                            {
                                Nuake::FileSystem::BeginWriteFile(pathCreation);
                                Nuake::FileSystem::WriteLine(TEMPLATE_SCRIPT_FIRST + fileName + TEMPLATE_SCRIPT_SECOND);
                                Nuake::FileSystem::EndWriteFile();

                                path = Nuake::FileSystem::AbsoluteToRelative(pathCreation);
                                Nuake::FileSystem::Scan();

                                component.LoadScript(path);
                                component.Script = path;
                            }
                            else
                            {
                                Nuake::Logger::Log("[FileSystem] Cannot create script files that starts with a number.", Nuake::CRITICAL);
                            }
                        }
                        
                    }
                }
                
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
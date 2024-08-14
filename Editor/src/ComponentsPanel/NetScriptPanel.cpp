#include "NetScriptPanel.h"
#include "../Windows/FileSystemUI.h"
#include <src/Scene/Components/NetScriptComponent.h>
#include <src/Core/FileSystem.h>
#include <src/Scripting/ScriptingEngineNet.h>
#include <src/Scene/Entities/ImGuiHelper.h>

void NetScriptPanel::Draw(Nuake::Entity entity)
{
    if (!entity.HasComponent<Nuake::NetScriptComponent>())
        return;

    auto& component = entity.GetComponent<Nuake::NetScriptComponent>();
    BeginComponentTable(.NETSCRIPT, Nuake::NetScriptComponent);
    {
        {
            ImGui::Text("Script");
            ImGui::TableNextColumn();

            std::string path = component.ScriptPath;
            ImGui::Button(path.empty() ? "Create New" : component.ScriptPath.c_str(), ImVec2(ImGui::GetContentRegionAvail().x, 0));
            if (ImGui::BeginDragDropTarget())
            {
                if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("_NetScript"))
                {
                    char* file = (char*)payload->Data;

                    std::string fullPath = std::string(file, 512);
                    path = Nuake::FileSystem::AbsoluteToRelative(std::move(fullPath));

                }
                ImGui::EndDragDropTarget();
            }

            component.ScriptPath = path;

            // Double click on file
            if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0))
            {
                if (!component.ScriptPath.empty())
                {
                    Nuake::OS::OpenIn(Nuake::FileSystem::Root + "component.ScriptPath");
                }
                else
                {
                    // TODO: Turn into command (Undo/Redo)
                    std::string pathCreation = Nuake::FileDialog::SaveFile("*.cs");

                    if (!pathCreation.empty())
                    {
                        if (!Nuake::String::EndsWith(pathCreation, ".cs"))
                        {
                            pathCreation += ".cs";
                        }

                        std::string fileName = Nuake::String::ToUpper(Nuake::FileSystem::GetFileNameFromPath(pathCreation));
                        fileName = Nuake::String::RemoveWhiteSpace(fileName);

                        if (!Nuake::String::IsDigit(fileName[0]))
                        {
                            Nuake::FileSystem::BeginWriteFile(pathCreation);
                            Nuake::FileSystem::WriteLine(NET_TEMPLATE_SCRIPT_FIRST + fileName + NET_TEMPLATE_SCRIPT_SECOND);
                            Nuake::FileSystem::EndWriteFile();

                            path = Nuake::FileSystem::AbsoluteToRelative(pathCreation);
                            Nuake::FileSystem::Scan();
                            Nuake::FileSystemUI::m_CurrentDirectory = Nuake::FileSystem::RootDirectory;
                        }
                        else
                        {
                            Nuake::Logger::Log("Cannot create script files that starts with a number.", "fileSystem", Nuake::CRITICAL);
                        }
                    }

                }
            }

            ImGui::TableNextColumn();

            ComponentTableReset(component.ScriptPath, "");
        }

        std::vector<std::string> detectedExposedVar;
        for (auto& e : Nuake::ScriptingEngineNet::Get().GetExposedVarForTypes(entity))
        {
            bool found = false;
            for (auto& c : component.ExposedVar)
            {
                if (e.Name == c.Name)
                {
                    c.Type = (Nuake::NetScriptExposedVarType)e.Type;
                    c.DefaultValue = e.Value;
                    found = true;
                }
            }

            detectedExposedVar.push_back(e.Name);

            if (!found)
            {
                Nuake::NetScriptExposedVar exposedVar;
                exposedVar.Name = e.Name;
                exposedVar.Value = e.Value;
                exposedVar.DefaultValue = e.Value;
                exposedVar.Type = (Nuake::NetScriptExposedVarType)e.Type;
                component.ExposedVar.push_back(exposedVar);
            }
        }

        // If we havent loaded the DLL, we cant trust the exposed var returned my the engine.
        if (Nuake::ScriptingEngineNet::Get().IsInitialized())
        {
            // Erase all exposed var from the component that dont exist in the DLL anymore.
            std::erase_if(component.ExposedVar,
                [&](Nuake::NetScriptExposedVar& var)
                {
                        return std::find(detectedExposedVar.begin(), detectedExposedVar.end(), var.Name) == detectedExposedVar.end();
                }
            );
        }

        for (auto& field : component.ExposedVar)
        {
            ImGui::TableNextColumn();
            {
                ImGui::Text(field.Name.c_str());
                ImGui::TableNextColumn();

                if (field.Type == Nuake::NetScriptExposedVarType::Float)
                {
                    if (!field.Value.has_value())
                    {
                        field.Value = field.DefaultValue;
                    }

                    float currentValue = std::any_cast<float>(field.Value);
                    const std::string sliderName = "##" + field.Name + "slider";
                    ImGui::DragFloat(sliderName.c_str(), &currentValue);
                    field.Value = currentValue;
                }

                if (field.Type == Nuake::NetScriptExposedVarType::Double)
                {
                    if (!field.Value.has_value())
                    {
                        field.Value = field.DefaultValue;
                    }

                    float currentValue = (float)std::any_cast<double>(field.Value);
                    const std::string sliderName = "##" + field.Name + "slider";
                    ImGui::DragFloat(sliderName.c_str(), &currentValue);
                    field.Value = (double)currentValue;
                }

                if (field.Type == Nuake::NetScriptExposedVarType::Bool)
                {
                    if (!field.Value.has_value())
                    {
                        field.Value = field.DefaultValue;
                    }

                    bool currentValue = std::any_cast<bool>(field.Value);
                    const std::string sliderName = "##" + field.Name + "slider";
                    ImGui::Checkbox(sliderName.c_str(), &currentValue);
                    field.Value = currentValue;
                }

                if (field.Type == Nuake::NetScriptExposedVarType::String)
                {
                    if (!field.Value.has_value())
                    {
                        field.Value = field.DefaultValue;
                    }

                    std::string currentValue = std::any_cast<std::string>(field.Value);
                    const std::string sliderName = "##" + field.Name + "slider";
                    ImGui::InputText(sliderName.c_str(), &currentValue);

                    field.Value = currentValue;
                }

                if (field.Type == Nuake::NetScriptExposedVarType::Vector2)
                {
                    if (!field.Value.has_value())
                    {
                        field.Value = field.DefaultValue;
                    }

                    
                    Nuake::Vector2 currentValue = std::any_cast<Nuake::Vector2>(field.Value);
                    const std::string sliderName = "##" + field.Name + "slider";
                    ImGuiHelper::DrawVec2(sliderName, &currentValue);

                    field.Value = currentValue;
                }

                if (field.Type == Nuake::NetScriptExposedVarType::Vector3)
                {
                    if (!field.Value.has_value())
                    {
                        field.Value = field.DefaultValue;
                    }

                    Nuake::Vector3 currentValue = std::any_cast<Nuake::Vector3>(field.Value);
                    const std::string sliderName = "##" + field.Name + "slider";
                    ImGuiHelper::DrawVec3(sliderName, &currentValue);

                    field.Value = currentValue;
                }

                ImGui::TableNextColumn();
            }
        }
        
        //    
        //    ImGui::TableNextColumn();
        //
        //    // Here we create a dropdown for every modules
        //    auto& wrenScript = component.mWrenScript;
        //    if (wrenScript)
        //    {
        //        auto modules = wrenScript->GetModules();
        //
        //        std::vector<const char*> modulesC;
        //
        //        for (auto& m : modules)
        //        {
        //            modulesC.push_back(m.c_str());
        //        }
        //        static int currentModule = (int)component.mModule;
        //        ImGui::Combo("##WrenModule", &currentModule, &modulesC[0], modules.size());
        //        component.mModule = currentModule;
        //
        //    ImGui::TableNextColumn();
        //    //ComponentTableReset(component.Class, "");
        //}
    }
    EndComponentTable();
}
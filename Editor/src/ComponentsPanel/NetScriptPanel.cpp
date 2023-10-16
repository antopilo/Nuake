#include "NetScriptPanel.h"
#include "../Windows/FileSystemUI.h"
#include <src/Scene/Components/NetScriptComponent.h>
#include <src/Core/FileSystem.h>

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
        //ImGui::TableNextColumn();
        //{
        //    ImGui::Text("Module");
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
        //    }
        //
        //    ImGui::TableNextColumn();
        //    //ComponentTableReset(component.Class, "");
        //}
    }
    EndComponentTable();
}
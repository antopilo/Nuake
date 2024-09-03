#include "ProjectInterface.h"
#include <src/Vendors/imgui/imgui.h>
#include "Engine.h"
#include "src/Core/FileSystem.h"
#include <src/Scripting/ScriptingEngineNet.h>
#include <src/Scripting/ScriptingEngineNet.h>
//#include "ImGuiTextHelper.h"

namespace Nuake {
    void ProjectInterface::DrawProjectSettings()
    {
        if (ImGui::Begin("Project settings"))
        {
            char buffer[256];
            memset(buffer, 0, sizeof(buffer));
            std::strncpy(buffer, Engine::GetProject()->Name.c_str(), sizeof(buffer));
            if (ImGui::InputText("##Name", buffer, sizeof(buffer)))
            {
                Engine::GetProject()->Name = std::string(buffer);
            }
        }
        ImGui::End();
    }

    void ProjectInterface::DrawCreatePointEntity()
    {
        char buffer[256];
        memset(buffer, 0, sizeof(buffer));
        std::strncpy(buffer, Engine::GetProject()->Name.c_str(), sizeof(buffer));
        if (ImGui::InputText("##Name", buffer, sizeof(buffer)))
        {
            Engine::GetProject()->Name = std::string(buffer);
        }
    }

    FGDPointEntity newEntity;

    const char* items[] = { "String", "Integer", "Float", "Boolean" };
    void ProjectInterface::DrawEntitySettings()
    {
        if (!m_CurrentProject)
            return;

        if (ImGui::Begin("Entity definitions"))
        {
            ImGui::Text("This is the entity definition used by trenchbroom. This files allows you to see your entities inside Trenchbroom");

            ImGui::Text("Trenchbroom path:");
            ImGui::SameLine();
            ImGui::Text(m_CurrentProject->TrenchbroomPath.c_str());
            ImGui::SameLine();
            if (ImGui::Button("Browse"))
            {
                std::string path = FileDialog::OpenFile("*.exe");
                if (path != "")
                {
                    path += "/../";
                    m_CurrentProject->TrenchbroomPath = path;
                }
            }

            Ref<FGDFile> file = Engine::GetProject()->EntityDefinitionsFile;

            auto flags = ImGuiWindowFlags_NoTitleBar;
            if (ImGui::BeginPopupModal("Create new point entity", NULL, flags))
            {
                //ImGuiTextSTD("Name", newEntity.Name);
                //ImGuiTextMultiline("Description", newEntity.Description);

                if (ImGui::BeginTable("DictCreate", 2, ImGuiTableFlags_Borders | ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable | ImGuiTableFlags_Hideable))
                {
                    ImGui::TableSetupColumn("Name");
                    ImGui::TableSetupColumn("Type");
                    ImGui::TableHeadersRow();

                    ImGui::TableNextColumn();
                    int idx = 0;
                    for (auto& p : newEntity.Properties)
                    {
                        //ImGuiTextSTD("Name", p.name);
                        ImGui::TableNextColumn();
                        std::string current_item = "";
                        if (ImGui::BeginCombo(("TypeSelection" + std::to_string(idx)).c_str(), current_item.c_str()))
                        {
                            for (int n = 0; n < IM_ARRAYSIZE(items); n++)
                            {
                                bool is_selected = (p.type == (ClassPropertyType)n); // You can store your selection however you want, outside or inside your objects
                                if (ImGui::Selectable(items[n], is_selected))
                                    if (is_selected)
                                    {
                                        p.type = (ClassPropertyType)n;
                                        ImGui::SetItemDefaultFocus();
                                    }

                            }
                            ImGui::EndCombo();
                        }
                        idx++;
                        ImGui::TableNextColumn();
                    }
                    if (ImGui::Button("Add new property"))
                    {
                        newEntity.Properties.push_back(ClassProperty());
                    }

                    ImGui::EndTable();
                }

                ImGui::Button("Create");
                ImGui::SameLine();
                if (ImGui::Button("Cancel"))
                    ImGui::CloseCurrentPopup();

                ImGui::EndPopup();
            }

            if (ImGui::Button("Scan from assembly"))
            {
                file->BrushEntities.clear();
                for (auto& [name, type] : Nuake::ScriptingEngineNet::Get().GetBrushEntities())
                {
                    FGDBrushEntity brushEntity = FGDBrushEntity(name);
                    brushEntity.Script = name;
                    brushEntity.Description = type.Description;
                    brushEntity.IsTrigger = type.isTrigger;
                    for (auto& t : type.exposedVars)
                    {
                        ClassProperty classProp;
                        classProp.name = t.Name;
                        classProp.type = ClassPropertyType::String;
                        if (t.Type == ExposedVarTypes::String && t.Value.has_value())
                        {
                            classProp.value = std::any_cast<std::string>(t.Value);
                        }
                        else if (t.Type == ExposedVarTypes::Int)
                        {
                            classProp.value = std::to_string(std::any_cast<int>(t.Value));
                        }
                        else
                        {
                            classProp.value = "";
                        }
                        brushEntity.Properties.push_back(classProp);
                    }

                    file->BrushEntities.push_back(brushEntity);
                }
            }

            if (ImGui::Button("Export"))
            {
                file->Export();
            }

            ImGui::SameLine();

            if (ImGui::Button("Save"))
            {
                file->Save();
            }

            ImGui::PushStyleVar(ImGuiStyleVar_WindowMinSize, ImVec2(0, 100));
            if (ImGui::BeginTabBar("##Tabs", ImGuiTabBarFlags_None))
            {
                if (ImGui::BeginTabItem("Point entities"))
                {
                    ImVec2 avail = ImGui::GetContentRegionAvail();
                    avail.y *= .8;

                    ImGui::BeginChild("table_child", avail, false);
                    if (ImGui::BeginTable("nested1", 4, ImGuiTableFlags_Borders | ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable | ImGuiTableFlags_Hideable))
                    {
                        ImGui::TableSetupColumn("Name");
                        ImGui::TableSetupColumn("Desciption");
                        ImGui::TableSetupColumn("Settings");
                        ImGui::TableSetupColumn("Prefab");
                        ImGui::TableHeadersRow();

                        ImGui::TableNextColumn();
                        int i = 0;
                        for (auto& pE : file->PointEntities)
                        {
                            //ImGuiTextSTD("##PName" + std::to_string(i), pE.Name);
                            for (int i = 0; i < pE.Name.size(); i++)
                            {
                                if (pE.Name[i] == ' ')
                                    pE.Name[i] = '_';
                            }
                            ImGui::TableNextColumn();
                            //ImGuiTextSTD("Desc" + std::to_string(i), pE.Description);
                            ImGui::TableNextColumn();
                            ImGui::Button("Edit");
                            ImGui::TableNextColumn();
                            //ImGuiTextSTD("Prefab##" + std::to_string(i), pE.Prefab);
                            if (ImGui::BeginDragDropTarget())
                            {
                                if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("_Prefab"))
                                {
                                    char* file = (char*)payload->Data;
                                    std::string fullPath = std::string(file, 256);
                                    pE.Prefab = FileSystem::AbsoluteToRelative(fullPath);
                                }
                                ImGui::EndDragDropTarget();
                            }
                            ImGui::TableNextColumn();
                            i++;
                        }

                        ImGui::EndTable();
                    }
                    ImGui::EndChild();

                    if (ImGui::Button("Add new"))
                    {
                        file->PointEntities.push_back(FGDPointEntity("NewPointEntity"));
                    }

                    ImGui::EndTabItem();
                }

                if (ImGui::BeginTabItem("Brush entities"))
                {
                    ImVec2 avail = ImGui::GetContentRegionAvail();
                    avail.y *= .8;
                    ImGui::BeginChild("table_child", avail, false);
                    if (ImGui::BeginTable("nested1", 4, ImGuiTableFlags_Borders | ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable | ImGuiTableFlags_Hideable))
                    {
                        ImGui::TableSetupColumn("Name");
                        ImGui::TableSetupColumn("Desciption");
                        ImGui::TableSetupColumn("Settings");
                        ImGui::TableSetupColumn("Script");
                        ImGui::TableHeadersRow();
                        ImGui::TableNextColumn();

                        int i = 0;
                        for (auto& pE : file->BrushEntities)
                        {
                            ImGui::Text(pE.Name.c_str());
                            //ImGuiTextSTD("##Name" + std::to_string(i), pE.Name);
                            ImGui::TableNextColumn();

                            //ImGuiTextSTD("Desc" + std::to_string(i), pE.Description);
                            ImGui::Text(pE.Description.c_str());
                            ImGui::TableNextColumn();

                            ImGui::Checkbox(std::string("Visible##" + std::to_string(i)).c_str(), &pE.Visible);
                            ImGui::SameLine();
                            ImGui::Checkbox(std::string("Solid##" + std::to_string(i)).c_str(), &pE.Solid);
                            ImGui::SameLine();
                            ImGui::Checkbox(std::string("Trigger##" + std::to_string(i)).c_str(), &pE.IsTrigger);

                            ImGui::TableNextColumn();

                            //ImGuiTextSTD("Script##" + std::to_string(i), pE.Script);
                            
                            //ImGuiTextSTD("Class##" + std::to_string(i), pE.Class);
                            ImGui::TableNextColumn();

                            i++;
                        }

                        ImGui::TableNextColumn();
                        ImGui::EndTable();

                        if (ImGui::BeginPopupModal("CreateBrush", NULL, flags))
                        {
                            //ImGuiTextSTD("Name", newEntity.Name);
                            //ImGuiTextMultiline("Description", newEntity.Description);

                            bool isSolid = true;
                            bool isTrigger = false;
                            bool isVisible = true;
                            ImGui::Checkbox("Is Solid", &isSolid);
                            ImGui::Checkbox("Is Trigger", &isTrigger);
                            ImGui::Checkbox("Is Visible", &isVisible);

                            if (ImGui::Button("Create"))
                            {

                            }

                            ImGui::SameLine();

                            if (ImGui::Button("Cancel"))
                                ImGui::CloseCurrentPopup();

                            ImGui::EndPopup();
                        }

                        if (ImGui::Button("Add new"))
                        {
                            file->BrushEntities.push_back(FGDBrushEntity("New brush"));
                        }

                    }
                    ImGui::EndChild();
                    ImGui::EndTabItem();
                }
                ImGui::EndTabBar();
            }

            ImGui::PopStyleVar();
        }
        ImGui::End();
    }
}

#include "ProjectInterface.h"
#include <src/Vendors/imgui/imgui.h>
#include "Engine.h"
#include "ImGuiTextHelper.h"

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

const char* items[] = { "String", "Integer", "Float", "Boolean"};
void ProjectInterface::DrawEntitySettings()
{
    if (ImGui::Begin("Entity definitions"))
    {
        ImGui::Text("This is the entity definition used by trenchbroom. This files allows you to see your entities inside Trenchbroom");
        ImGui::Text("Trenchbroom path:");

        Ref<FGDFile> file = Engine::GetProject()->EntityDefinitionsFile;


        auto flags = ImGuiWindowFlags_NoTitleBar;
        if (ImGui::BeginPopupModal("Create new point entity", NULL, flags))
        {
            ImGuiTextSTD("Name", newEntity.Name);
            ImGuiTextMultiline("Description", newEntity.Description);
            
            if (ImGui::BeginTable("DictCreate", 2, ImGuiTableFlags_Borders | ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable | ImGuiTableFlags_Hideable))
            {
                ImGui::TableSetupColumn("Name");
                ImGui::TableSetupColumn("Type");
                ImGui::TableHeadersRow();

                ImGui::TableNextColumn();
                int idx = 0;
                for (auto& p : newEntity.Properties)
                {
                    ImGuiTextSTD("Name", p.name);
                    ImGui::TableNextColumn();
                    std::string current_item = NULL;
                    if(ImGui::BeginCombo(("TypeSelection" + std::to_string(idx)).c_str(), current_item.c_str()))
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
                if (ImGui::Button("Add new property")) {
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
                    for (auto& pE : file->PointEntities)
                    {
                        ImGui::Text(pE.Name.c_str());
                        ImGui::TableNextColumn();
                        ImGui::Text(pE.Description.c_str());
                        ImGui::TableNextColumn();
                        ImGui::Button("Edit");
                        ImGui::TableNextColumn();
                        ImGui::Text(pE.Prefab.c_str());
                        ImGui::SameLine();
                        ImGui::Button("Browse");
                    }

                    ImGui::EndTable();
                    
                }
                ImGui::EndChild();
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Brush entities"))
            {
                ImGui::Text("ID: 0123456789");
                ImGui::EndTabItem();
            }
            ImGui::EndTabBar();
        }
       
        ImGui::PopStyleVar();

        if (ImGui::Button("Add new"))
            ImGui::OpenPopup("Create new point entity");
    }
    ImGui::End();
}
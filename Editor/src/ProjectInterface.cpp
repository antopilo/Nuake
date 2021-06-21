#include "ProjectInterface.h"
#include <src/Vendors/imgui/imgui.h>
#include "Engine.h"

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


void ProjectInterface::DrawEntitySettings()
{
    if (ImGui::Begin("Entity definitions"))
    {
        ImGui::Text("This is the entity definition used by trenchbroom. This files allows you to see your entities inside Trenchbroom");
        ImGui::Text("Trenchbroom path:");
        // path here...
        Ref<FGDFile> file = Engine::GetProject()->EntityDefinitionsFile;



        auto flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize;
        if (ImGui::BeginPopupModal("Create new point entity", NULL, flags))
        {
        

            ImGui::Button("Create");
            ImGui::SameLine();
            ImGui::Button("Cancel");

            ImGui::EndPopup();
        }
        if (ImGui::BeginTabBar("##Tabs", ImGuiTabBarFlags_None))
        {
            if (ImGui::BeginTabItem("Point entities"))
            {
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
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Brush entities"))
            {
                ImGui::Text("ID: 0123456789");
                ImGui::EndTabItem();
            }
            ImGui::EndTabBar();
        }
        
        

    }
    ImGui::End();
}
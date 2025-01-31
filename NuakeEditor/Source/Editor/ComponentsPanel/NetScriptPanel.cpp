#include "NetScriptPanel.h"

#include "../Windows/FileSystemUI.h"

#include <Nuake/FileSystem/FileDialog.h>
#include "Nuake/FileSystem/FileSystem.h"
#include <Nuake/Scripting/ScriptingEngineNet.h>
#include <Nuake/Scene/Components/NetScriptComponent.h>
#include <Nuake/Scene/Entities/ImGuiHelper.h>

const std::string NET_TEMPLATE_SCRIPT_FIRST = R"(using Nuake.Net;

namespace NuakeShowcase
{
    class )";

const std::string NET_TEMPLATE_SCRIPT_SECOND = R"( : Entity
    {
        public override void OnInit()
        {
            // Called once at the start of the game
        }

        
        public override void OnUpdate(float dt)
        {
            // Called every frame
        }
        
        public override void OnFixedUpdate(float dt)
        {
            // Called every fixed update
        }

        
        public override void OnDestroy()
        {
            // Called at the end of the game fixed update
        }
    }
} 
)";

void NetScriptPanel::Draw(Nuake::Entity& entity, entt::meta_any& componentInstance)
{
    using namespace Nuake;
        
    Nuake::NetScriptComponent* componentPtr = componentInstance.try_cast<Nuake::NetScriptComponent>();
    if (componentPtr == nullptr)
    {
        return;
    }
    Nuake::NetScriptComponent& component = *componentPtr;
    
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
                        if (!field.DefaultValue.has_value())
                        {
                            field.DefaultValue = 0.0f;
                        }

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
                        if (!field.DefaultValue.has_value())
                        {
                            field.DefaultValue = 0.0;
                        }

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
                        if (!field.DefaultValue.has_value())
                        {
                            field.DefaultValue = false;
                        }

                        field.Value = field.DefaultValue;
                    }

                     auto typeName = field.Value.type().name();
                    if (typeName == std::string("bool"))
                    {
                        bool currentValue = std::any_cast<bool>(field.Value);
                        const std::string sliderName = "##" + field.Name + "slider";
                        ImGui::Checkbox(sliderName.c_str(), &currentValue);
                        field.Value = currentValue;
                    }
                }

                if (field.Type == Nuake::NetScriptExposedVarType::String)
                {
                    if (!field.Value.has_value())
                    {
                        if (!field.DefaultValue.has_value())
                        {
                            field.DefaultValue = std::string();
                        }

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
                        if (!field.DefaultValue.has_value())
                        {
                            field.DefaultValue = Nuake::Vector2(0, 0);
                        }

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
                        if (!field.DefaultValue.has_value())
                        {
                            field.DefaultValue = Nuake::Vector3(0, 0, 0);
                        }

                        field.Value = field.DefaultValue;
                    }

                    Nuake::Vector3 currentValue = std::any_cast<Nuake::Vector3>(field.Value);
                    const std::string sliderName = "##" + field.Name + "slider";
                    ImGuiHelper::DrawVec3(sliderName, &currentValue);

                    field.Value = currentValue;
                }

                if (field.Type == Nuake::NetScriptExposedVarType::Entity)
                {
                    if (!field.Value.has_value() && field.DefaultValue.has_value())
                    {
                        field.Value = field.DefaultValue;
                    }

                    int entityId = -1;
                    if (field.Value.has_value())
                    {
                        entityId = std::any_cast<int>(field.Value);
                    }

                    auto entity = Nuake::Engine::GetCurrentScene()->GetEntityByID(entityId);
                    bool invalid = !entity.IsValid();
                    std::string buttonLabel;
                    if (entityId == -1)
                    {
                        buttonLabel = "No Entity";
                    }
                    else if (invalid)
                    {
                        buttonLabel = "Invalid Entity";
                    }
                    else
                    {
                        buttonLabel = entity.GetComponent<Nuake::NameComponent>().Name;
                    }

                    ImGui::Button(buttonLabel.c_str(), ImVec2(ImGui::GetContentRegionAvail().x, 0));

                    if (ImGui::BeginDragDropTarget())
                    {
                        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ENTITY"))
                        {
                            Nuake::Entity payload_entity = *(const Nuake::Entity*)payload->Data;

                            field.Value = payload_entity.GetID();
                        }
                        ImGui::EndDragDropTarget();
                    }

                }

                if (field.Type == Nuake::NetScriptExposedVarType::Prefab)
                {
                    if (!field.Value.has_value() && field.DefaultValue.has_value())
                    {
                        field.Value = field.DefaultValue;
                    }

                    std::string prefabPath = "";
                    if (field.Value.has_value())
                    {
                        prefabPath = std::any_cast<std::string>(field.Value);
                    }

                    std::string buttonLabel;
                    if (!Nuake::FileSystem::FileExists(prefabPath))
                    {
                        buttonLabel = "File doesn't exist";
                    }
                    else if (prefabPath.empty())
                    {
                        buttonLabel = "Empty";
                    }
                    else
                    {
                        buttonLabel = prefabPath;
                    }

                    if (ImGui::Button(buttonLabel.c_str(), ImVec2(ImGui::GetContentRegionAvail().x, 0)))
                    {
                        const std::string& newPath = Nuake::FileDialog::OpenFile("Prefab file\0*.prefab");

                        const std::string& relativePath = Nuake::FileSystem::AbsoluteToRelative(newPath);
                        if (Nuake::FileSystem::FileExists(relativePath))
                        {
                            field.Value = relativePath;
                        }
                    }

                    if (ImGui::BeginDragDropTarget())
                    {
                        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("_Prefab"))
                        {
                            char* file = (char*)payload->Data;
                            const std::string filePath = Nuake::FileSystem::AbsoluteToRelative(std::string(file, 512));
                            if (Nuake::FileSystem::FileExists(filePath))
                            {
                                field.Value = filePath;
                            }
                        }
                        ImGui::EndDragDropTarget();
                    }
                }

                ImGui::TableNextColumn();

                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1, 1, 1, 0));
                std::string resetLabel = std::string(ICON_FA_UNDO) + "##Reset" + field.Name;
                if (ImGui::Button(resetLabel.c_str()))
                {
                    field.Value = field.DefaultValue;
                }
                ImGui::PopStyleColor();
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
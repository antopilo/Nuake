#include "EditorSelectionPanel.h"
#include "src/Scene/Components/ImportComponents.h"
#include "../Misc/ImGuiTextHelper.h"

EditorSelectionPanel::EditorSelectionPanel()
{
    mTransformPanel = TransformPanel();
    mLightPanel = LightPanel();
    mScriptPanel = ScriptPanel();
    mQuakeMapPanel = QuakeMapPanel();
}

void EditorSelectionPanel::Draw(EditorSelection selection)
{
    if (ImGui::Begin("Propreties"))
    {
        switch (selection.Type)
        {
            case EditorSelectionType::None:
            {
                DrawNone();
                break;
            }

            case EditorSelectionType::Entity:
            {
                DrawEntity(selection.Entity);
                break;
            }
            case EditorSelectionType::File:
            {
                DrawFile(selection.File.get());
                break;
            }
            case EditorSelectionType::Resource:
            {
                DrawResource(selection.Resource);
                break;
            }
        }
    }
    ImGui::End();
}

void EditorSelectionPanel::DrawNone()
{
    std::string text = "No selection";
    auto windowWidth = ImGui::GetWindowSize().x;
    auto windowHeight = ImGui::GetWindowSize().y;

    auto textWidth = ImGui::CalcTextSize(text.c_str()).x;
    auto textHeight = ImGui::CalcTextSize(text.c_str()).y;
    ImGui::SetCursorPosX((windowWidth - textWidth) * 0.5f);
    ImGui::SetCursorPosY((windowHeight - textHeight) * 0.5f);

    ImGui::Text(text.c_str());
}

void EditorSelectionPanel::DrawEntity(Nuake::Entity entity)
{
    DrawAddComponentMenu(entity);

    // Draw each component properties panels.
    mTransformPanel.Draw(entity);
    mLightPanel.Draw(entity);
    mScriptPanel.Draw(entity);
    mMeshPanel.Draw(entity);
    mQuakeMapPanel.Draw(entity);
    /*
    if (Selection.Entity.HasComponent<MeshComponent>())
    {
        std::string icon = ICON_FA_MALE;
        if (ImGui::CollapsingHeader((icon + " " + "Mesh").c_str(), ImGuiTreeNodeFlags_DefaultOpen))
        {
            ImGui::TextColored(ImGui::GetStyleColorVec4(1), "Mesh properties");
            auto& component = Selection.Entity.GetComponent<MeshComponent>();
            // Path
            std::string path = component.ModelPath;
            char pathBuffer[256];

            memset(pathBuffer, 0, sizeof(pathBuffer));
            std::strncpy(pathBuffer, path.c_str(), sizeof(pathBuffer));

            std::string oldPath = component.ModelPath;
            ImGui::Text("Model: ");
            ImGui::SameLine();
            if (ImGui::InputText("##ModelPath", pathBuffer, sizeof(pathBuffer)))
                path = FileSystem::AbsoluteToRelative(std::string(pathBuffer));
            if (ImGui::BeginDragDropTarget())
            {
                if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("_Model"))
                {
                    char* file = (char*)payload->Data;
                    std::string fullPath = std::string(file, 256);
                    path = FileSystem::AbsoluteToRelative(fullPath);
                }
                ImGui::EndDragDropTarget();
            }

            if (component.ModelPath != path)
            {
                component.ModelPath = path;
                component.LoadModel();
            }

            ImGui::SameLine();

            if (ImGui::Button("Reimport"))
            {
                component.LoadModel();
            }

            ImGui::Indent(16.0f);
            if (ImGui::CollapsingHeader("Meshes"))
            {
                ImGui::Indent(16.0f);
                uint16_t index = 0;
                for (auto& m : component.meshes)
                {
                    if (ImGui::CollapsingHeader(std::to_string(index).c_str()))
                    {
                        std::string materialName = "No material";
                        if (m->m_Material)
                            materialName = m->m_Material->GetName();

                        ImGui::Indent(16.0f);
                        if (ImGui::CollapsingHeader(materialName.c_str()))
                        {
                            //if (ImGui::BeginChild("Material child", ImVec2(0, 0), true, ImGuiWindowFlags_AlwaysAutoResize))
                            //{
                            ImGui::Indent(16.0f);
                            DrawMaterialEditor(m->m_Material);
                            //}
                            //ImGui::EndChild();
                        }

                        if (ImGui::BeginDragDropTarget())
                        {
                            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("_Material"))
                            {
                                char* file = (char*)payload->Data;
                                std::string fullPath = std::string(file, 256);
                                path = FileSystem::AbsoluteToRelative(fullPath);
                            }
                            ImGui::EndDragDropTarget();
                        }
                    }

                    index++;
                }

            }

            ImGui::Separator();
        }

    }

    

    if (Selection.Entity.HasComponent<WrenScriptComponent>()) {
        std::string icon = ICON_FA_FILE;
        if (ImGui::CollapsingHeader((icon + " " + "Wren Script").c_str(), ImGuiTreeNodeFlags_DefaultOpen))
        {
            ImGui::TextColored(ImGui::GetStyleColorVec4(1), "Script properties");
            auto& component = Selection.Entity.GetComponent<WrenScriptComponent>();

            // Path
            std::string path = component.Script;
            char pathBuffer[256];

            memset(pathBuffer, 0, sizeof(pathBuffer));
            std::strncpy(pathBuffer, path.c_str(), sizeof(pathBuffer));

            ImGui::Text("Script: ");
            ImGui::SameLine();
            if (ImGui::InputText("##ScriptPath", pathBuffer, sizeof(pathBuffer)))
                path = FileSystem::AbsoluteToRelative(std::string(pathBuffer));
            if (ImGui::BeginDragDropTarget())
            {
                if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("_Script"))
                {
                    char* file = (char*)payload->Data;
                    std::string fullPath = std::string(file, 256);
                    path = FileSystem::AbsoluteToRelative(fullPath);
                }
                ImGui::EndDragDropTarget();
            }


            component.Script = path;

            // Class
            std::string module = component.Class;

            char classBuffer[256];

            memset(classBuffer, 0, sizeof(classBuffer));
            std::strncpy(classBuffer, module.c_str(), sizeof(classBuffer));

            ImGui::Text("Class: ");
            ImGui::SameLine();
            if (ImGui::InputText("##ScriptModule", classBuffer, sizeof(classBuffer)))
                module = std::string(classBuffer);

            component.Class = module;
            ImGui::Separator();
        }
    }

    if (Selection.Entity.HasComponent<CameraComponent>()) {

        std::string icon = ICON_FA_LIGHTBULB;
        if (ImGui::CollapsingHeader((icon + " " + "Camera").c_str(), ImGuiTreeNodeFlags_DefaultOpen))
        {
            ImGui::TextColored(ImGui::GetStyleColorVec4(1), "Camera properties");
            Selection.Entity.GetComponent<CameraComponent>().DrawEditor();
            ImGui::Separator();
        }

    }

    if (Selection.Entity.HasComponent<CharacterControllerComponent>())
    {
        if (ImGui::CollapsingHeader("Character controller", ImGuiTreeNodeFlags_DefaultOpen))
        {
            ImGui::TextColored(ImGui::GetStyleColorVec4(1), "Character controller properties");
            auto& c = Selection.Entity.GetComponent<CharacterControllerComponent>();
            ImGui::InputFloat("Height", &c.Height);
            ImGui::InputFloat("Radius", &c.Radius);
            ImGui::InputFloat("Mass", &c.Mass);
            ImGui::Separator();
        }
    }
    if (Selection.Entity.HasComponent<RigidBodyComponent>())
    {
        std::string icon = ICON_FA_BOWLING_BALL;
        if (ImGui::CollapsingHeader((icon + " Rigidbody").c_str(), ImGuiTreeNodeFlags_DefaultOpen))
        {
            ImGui::TextColored(ImGui::GetStyleColorVec4(1), "Rigidbody properties");
            RigidBodyComponent& rbComponent = Selection.Entity.GetComponent<RigidBodyComponent>();
            ImGui::DragFloat("Mass", &rbComponent.mass, 0.1, 0.0);
            ImGui::Separator();
        }
    }
    if (Selection.Entity.HasComponent<BoxColliderComponent>())
    {
        std::string icon = ICON_FA_BOX;
        if (ImGui::CollapsingHeader((icon + " Box collider").c_str(), ImGuiTreeNodeFlags_DefaultOpen))
        {
            ImGui::TextColored(ImGui::GetStyleColorVec4(1), "Box collider properties");
            BoxColliderComponent& component = Selection.Entity.GetComponent<BoxColliderComponent>();
            ImGuiHelper::DrawVec3("Size", &component.Size);
            ImGui::Checkbox("Is trigger", &component.IsTrigger);

            ImGui::Separator();
        }
    }
    if (Selection.Entity.HasComponent<SphereColliderComponent>())
    {
        std::string icon = ICON_FA_CIRCLE;
        if (ImGui::CollapsingHeader((icon + " Sphere collider").c_str(), ImGuiTreeNodeFlags_DefaultOpen))
        {
            ImGui::TextColored(ImGui::GetStyleColorVec4(1), "Sphere properties");
            SphereColliderComponent& component = Selection.Entity.GetComponent<SphereColliderComponent>();
            ImGui::DragFloat("Radius", &component.Radius, 0.1f, 0.0f, 100.0f);
            ImGui::Checkbox("Is trigger", &component.IsTrigger);

            ImGui::Separator();
        }
    }
    if (Selection.Entity.HasComponent<QuakeMapComponent>())
    {

        std::string icon = ICON_FA_BROOM;
        if (ImGui::CollapsingHeader((icon + " " + "Quake map").c_str(), ImGuiTreeNodeFlags_DefaultOpen))
        {
            ImGui::TextColored(ImGui::GetStyleColorVec4(1), "Quake map properties");
            auto& component = Selection.Entity.GetComponent<QuakeMapComponent>();
            std::string path = component.Path;


            char pathBuffer[256];
            memset(pathBuffer, 0, sizeof(pathBuffer));
            std::strncpy(pathBuffer, path.c_str(), sizeof(pathBuffer));
            ImGui::Text("Map file: ");
            ImGui::SameLine();
            if (ImGui::InputText("##MapPath", pathBuffer, sizeof(pathBuffer)))
            {
                path = std::string(pathBuffer);
            }

            if (ImGui::BeginDragDropTarget())
            {
                if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("_Map"))
                {
                    char* file = (char*)payload->Data;
                    std::string fullPath = std::string(file, 256);
                    path = FileSystem::AbsoluteToRelative(fullPath);
                }
                ImGui::EndDragDropTarget();
            }

            component.Path = path;

            ImGui::InputFloat("Scale factor", &component.ScaleFactor, 0.01f, 0.1f);

            ImGui::Checkbox("Build collisions", &component.HasCollisions);
            if (ImGui::Button("Build Geometry"))
            {
                QuakeMapBuilder mapBuilder;
                mapBuilder.BuildQuakeMap(Selection.Entity);
            }
            ImGui::Separator();
        }
    }*/
}

void EditorSelectionPanel::DrawAddComponentMenu(Nuake::Entity entity)
{
    if (entity.HasComponent<Nuake::NameComponent>())
    {
        auto& entityName = entity.GetComponent<Nuake::NameComponent>().Name;
        ImGuiTextSTD("##Name", entityName);
        ImGui::SameLine();

        if (ImGui::Button("Add Component"))
            ImGui::OpenPopup("ComponentPopup");

        if (ImGui::BeginPopup("ComponentPopup"))
        {
            MenuItemComponent("Wren Script", Nuake::WrenScriptComponent);
            MenuItemComponent("Camera", Nuake::CameraComponent);
            MenuItemComponent("Light", Nuake::LightComponent);
            MenuItemComponent("Mesh", Nuake::MeshComponent);
            MenuItemComponent("Rigid body", Nuake::RigidBodyComponent);
            MenuItemComponent("Box collider", Nuake::BoxColliderComponent);
            MenuItemComponent("Sphere collider", Nuake::SphereColliderComponent);
            MenuItemComponent("Mesh collider", Nuake::MeshColliderComponent);
            MenuItemComponent("Quake map", Nuake::QuakeMapComponent);
            ImGui::EndPopup();
        }
        ImGui::Separator();
    }
    
}

void EditorSelectionPanel::DrawFile(Nuake::File* file)
{

}

void EditorSelectionPanel::DrawResource(Nuake::Resource resource)
{

}


#include "EditorSelectionPanel.h"
#include "../Misc/ImGuiTextHelper.h"
#include <src/Scene/Components/Components.h>

#include <src/Rendering/Textures/Material.h>
#include <src/Resource/ResourceLoader.h>
#include <src/Resource/FontAwesome5.h>
#include <Engine.h>

EditorSelectionPanel::EditorSelectionPanel()
{
    mTransformPanel = TransformPanel();
    mLightPanel = LightPanel();
    mScriptPanel = ScriptPanel();
    mQuakeMapPanel = QuakeMapPanel();
}

void EditorSelectionPanel::ResolveFile(Ref<Nuake::File> file)
{
    using namespace Nuake;

    currentFile = file;

    if (currentFile->GetExtension() == ".project")
    {

    }

    if (currentFile->GetExtension() == ".material")
    {
        Ref<Material> material = ResourceLoader::LoadMaterial(currentFile->GetRelativePath());
        selectedResource = material;
    }
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
                if (currentFile != selection.File)
                {
                    ResolveFile(selection.File);
                }

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
            MenuItemComponent("Model", Nuake::ModelComponent);
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
    using namespace Nuake;
    if (file->GetExtension() == ".material")
    {
        DrawMaterialPanel(std::static_pointer_cast<Material>(selectedResource));
    }
    if (file->GetExtension() == ".project")
    {
        DrawProjectPanel(Nuake::Engine::GetProject());
    }
}

void EditorSelectionPanel::DrawResource(Nuake::Resource resource)
{

}

void EditorSelectionPanel::DrawMaterialPanel(Ref<Nuake::Material> material)
{
    using namespace Nuake;

    std::string materialTitle = material->Path;
    {
        UIFont boldfont = UIFont(Fonts::SubTitle);
        ImGui::Text(material->Path.c_str());

    }
    ImGui::SameLine();
    {
        UIFont boldfont = UIFont(Fonts::Icons);
        if (ImGui::Button(ICON_FA_SAVE))
        {
            std::string fileData = material->Serialize().dump(4);

            FileSystem::BeginWriteFile(FileSystem::Root + material->Path);
            FileSystem::WriteLine(fileData);
            FileSystem::EndWriteFile();
        }
    }

    bool flagsHeaderOpened;
    {
		UIFont boldfont = UIFont(Fonts::Bold); 
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.f, 0.f));
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.f, 8.f));
		flagsHeaderOpened = ImGui::CollapsingHeader(" FLAGS", ImGuiTreeNodeFlags_DefaultOpen);
		ImGui::PopStyleVar(2);
    }
	
    if (flagsHeaderOpened)
    {
		ImGui::BeginTable("##Flags", 3, ImGuiTableFlags_BordersInner);
		{
			ImGui::TableSetupColumn("name", 0, 0.3f);
			ImGui::TableSetupColumn("set", 0, 0.6f);
			ImGui::TableSetupColumn("reset", 0, 0.1f);
			ImGui::TableNextColumn();

			ImGui::Text("Unlit");
			ImGui::TableNextColumn();

			bool unlit = material->data.u_Unlit == 1;
			ImGui::Checkbox("Unlit", &unlit);
			material->data.u_Unlit = (int)unlit;
		}
		ImGui::EndTable();
    }

	const auto TexturePanelHeight = 100;
    const ImVec2 TexturePanelSize = ImVec2(0, TexturePanelHeight);
    bool AlbedoOpened;
	{
		UIFont boldfont = UIFont(Fonts::Bold);
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.f, 0.f));
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.f, 8.f));
        AlbedoOpened = ImGui::CollapsingHeader("Albedo", ImGuiTreeNodeFlags_DefaultOpen);
		ImGui::PopStyleVar(2);
	}

	if (AlbedoOpened)
	{
        ImGui::BeginChild("##albedo", TexturePanelSize, true);
        {
			uint32_t textureID = 0;
			if (material->HasAlbedo())
			{
				textureID = material->m_Albedo->GetID();
			}
			else
			{
				textureID = TextureManager::Get()->GetTexture("default")->GetID();
			}

			if (ImGui::ImageButtonEx(ImGui::GetCurrentWindow()->GetID("#image1"), (void*)textureID, ImVec2(80, 80), ImVec2(0, 1), ImVec2(1, 0), ImVec2(2, 2), ImVec4(0, 0, 0, 0), ImVec4(1, 1, 1, 1)))
			{
				std::string texture = FileDialog::OpenFile("*.png | *.jpg");
				if (texture != "")
				{
					material->SetAlbedo(TextureManager::Get()->GetTexture(texture));
				}
			}

			if (ImGui::BeginPopupContextWindow())
			{
				if (ImGui::MenuItem("Clear Texture"))
				{
					material->m_Albedo = nullptr;
				}
				ImGui::EndPopup();
			}

			ImGui::SameLine();
			ImGui::ColorEdit3("Color", &material->data.m_AlbedoColor.r);
        }
        ImGui::EndChild();
	}

	if (ImGui::CollapsingHeader("Normal", ImGuiTreeNodeFlags_DefaultOpen))
	{
        ImGui::BeginChild("##normal", TexturePanelSize, true);
        {
			uint32_t textureID = 0;
			if (material->HasNormal())
			{
				textureID = material->m_Normal->GetID();
			}
			else
			{
				textureID = TextureManager::Get()->GetTexture("default")->GetID();
			}

			if (ImGui::ImageButtonEx(ImGui::GetCurrentWindow()->GetID("#image3"), (void*)textureID, ImVec2(80, 80), ImVec2(0, 1), ImVec2(1, 0), ImVec2(2, 2), ImVec4(0, 0, 0, 1), ImVec4(1, 1, 1, 1)))
			{
				std::string texture = FileDialog::OpenFile("*.png | *.jpg");
				if (texture != "")
				{
					material->SetNormal(TextureManager::Get()->GetTexture(texture));
				}
			}

			if (ImGui::BeginPopupContextWindow())
			{
				if (ImGui::MenuItem("Clear Texture"))
				{
					material->m_Normal = nullptr;
				}
				ImGui::EndPopup();
			}
        }
        ImGui::EndChild();
	}

	if (ImGui::CollapsingHeader("AO", ImGuiTreeNodeFlags_DefaultOpen))
	{
        ImGui::BeginChild("##ao", TexturePanelSize, true);
        {
			uint32_t textureID = 0;
			if (material->HasAO())
			{
				textureID = material->m_AO->GetID();
			}
			else
			{
				textureID = TextureManager::Get()->GetTexture("default")->GetID();
			}

			if (ImGui::ImageButtonEx(ImGui::GetCurrentWindow()->GetID("#image2"), (void*)textureID, ImVec2(80, 80), ImVec2(0, 1), ImVec2(1, 0), ImVec2(2, 2), ImVec4(1, 1, 1, 1), ImVec4(1, 1, 1, 1)))
			{
				std::string texture = FileDialog::OpenFile("Image files (*.png) | *.png | Image files (*.jpg) | *.jpg");
				if (texture != "")
				{
					material->SetAO(TextureManager::Get()->GetTexture(texture));
				}
			}

			if (ImGui::BeginPopupContextWindow())
			{
				if (ImGui::MenuItem("Clear Texture"))
				{
					material->m_AO = nullptr;
				}
				ImGui::EndPopup();
			}
        }
        ImGui::EndChild();
	}
	
	if (ImGui::CollapsingHeader("Metalness", ImGuiTreeNodeFlags_DefaultOpen))
	{
        ImGui::BeginChild("##metalness", TexturePanelSize, true);
        {
			uint32_t textureID = 0;
			if (material->HasMetalness())
			{
				textureID = material->m_Metalness->GetID();
			}
			else
			{
				textureID = TextureManager::Get()->GetTexture("default")->GetID();
			}

			if (ImGui::ImageButtonEx(ImGui::GetCurrentWindow()->GetID("#image4"), (void*)textureID, ImVec2(80, 80), ImVec2(0, 1), ImVec2(1, 0), ImVec2(2, 2), ImVec4(0, 0, 0, 1), ImVec4(1, 1, 1, 1)))
			{
				std::string texture = FileDialog::OpenFile("*.png | *.jpg");
				if (texture != "")
				{
					material->SetMetalness(TextureManager::Get()->GetTexture(texture));
				}
			}

			if (ImGui::BeginPopupContextWindow())
			{
				if (ImGui::MenuItem("Clear Texture"))
				{
					material->m_Metalness = nullptr;
				}
				ImGui::EndPopup();
			}

			ImGui::SameLine();
			ImGui::DragFloat("Value##4", &material->data.u_MetalnessValue, 0.01f, 0.0f, 1.0f);
        }
        ImGui::EndChild();
	}

	if (ImGui::CollapsingHeader("Roughness", ImGuiTreeNodeFlags_DefaultOpen))
	{
        ImGui::BeginChild("##roughness", TexturePanelSize, true);
        {
			uint32_t textureID = 0;
			if (material->HasRoughness())
			{
				textureID = material->m_Roughness->GetID();
			}
			else
			{
				textureID = TextureManager::Get()->GetTexture("default")->GetID();
			}

			if (ImGui::ImageButtonEx(ImGui::GetCurrentWindow()->GetID("#image5"), (void*)textureID, ImVec2(80, 80), ImVec2(0, 1), ImVec2(1, 0), ImVec2(2, 2), ImVec4(0, 0, 0, 0), ImVec4(1, 1, 1, 1)))
			{
				std::string texture = FileDialog::OpenFile("*.png | *.jpg");
				if (texture != "")
				{
					material->SetRoughness(TextureManager::Get()->GetTexture(texture));
				}
			}

			if (ImGui::BeginPopupContextWindow())
			{
				if (ImGui::MenuItem("Clear Texture"))
				{
					material->m_Roughness = nullptr;
				}
				ImGui::EndPopup();
			}
        }
        ImGui::EndChild();
	}
}

void EditorSelectionPanel::DrawProjectPanel(Ref<Nuake::Project> project)
{
    ImGui::InputText("Project Name", &project->Name);
    ImGui::InputTextMultiline("Project Description", &project->Description);

    if (ImGui::Button("Locate"))
    {

    }
    ImGui::SameLine();
    ImGui::InputText("Trenchbroom Path", &project->TrenchbroomPath);
}


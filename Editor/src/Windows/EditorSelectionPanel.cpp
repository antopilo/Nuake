#include "EditorSelectionPanel.h"
#include "../Misc/ImGuiTextHelper.h"
#include <src/Scene/Components/Components.h>

#include <src/Rendering/Textures/Material.h>
#include <src/Resource/ResourceLoader.h>
#include <src/Resource/FontAwesome5.h>
#include <src/Scripting/WrenScript.h>
#include <Engine.h>

EditorSelectionPanel::EditorSelectionPanel()
{
    mTransformPanel = TransformPanel();
    mLightPanel = LightPanel();
    mScriptPanel = ScriptPanel();
    mQuakeMapPanel = QuakeMapPanel();
    mCameraPanel = CameraPanel();
    mRigidbodyPanel = RigidbodyPanel();
    mBoxColliderPanel = BoxColliderPanel();
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
    if (ImGui::Begin("Properties"))
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

                DrawFile(selection.File);
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
    mCameraPanel.Draw(entity);
    mRigidbodyPanel.Draw(entity);
    mBoxColliderPanel.Draw(entity);
    mSphereColliderPanel.Draw(entity);
	mCapsuleColliderPanel.Draw(entity);
	mCylinderColliderPanel.Draw(entity);
    mMeshColliderPanel.Draw(entity);
    mCharacterControllerPanel.Draw(entity);
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
			MenuItemComponent("Wren Script", Nuake::WrenScriptComponent)
			MenuItemComponent("Camera", Nuake::CameraComponent)
			MenuItemComponent("Light", Nuake::LightComponent)
			MenuItemComponent("Model", Nuake::ModelComponent)
			ImGui::Separator();
			MenuItemComponent("Character Controller", Nuake::CharacterControllerComponent)
            MenuItemComponent("Rigid body", Nuake::RigidBodyComponent)
			ImGui::Separator();
            MenuItemComponent("Box collider", Nuake::BoxColliderComponent)
			MenuItemComponent("Capsule collider", Nuake::CapsuleColliderComponent)
			MenuItemComponent("Cylinder collider", Nuake::CylinderColliderComponent)
            MenuItemComponent("Sphere collider", Nuake::SphereColliderComponent)
            MenuItemComponent("Mesh collider", Nuake::MeshColliderComponent)
			ImGui::Separator();
            MenuItemComponent("Quake map", Nuake::QuakeMapComponent)
            ImGui::EndPopup();
        }
        ImGui::Separator();
    }
    
}

void EditorSelectionPanel::DrawFile(Ref<Nuake::File> file)
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
	if (file->GetExtension() == ".wren")
	{
		DrawWrenScriptPanel(CreateRef<WrenScript>(file, true));
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

void EditorSelectionPanel::DrawWrenScriptPanel(Ref<Nuake::WrenScript> wrenFile)
{
	auto filePath = wrenFile->GetFile()->GetAbsolutePath();
	std::string fileContent = Nuake::FileSystem::ReadFile(filePath, true);

	ImGui::Text("Content");
	ImGui::Separator();
	ImGui::PushTextWrapPos(ImGui::GetCursorPos().x + ImGui::GetWindowWidth());
	ImGui::Text(fileContent.c_str(), ImGui::GetWindowWidth());
	
	ImGui::PopTextWrapPos();
}


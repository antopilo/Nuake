#include "MaterialEditor.h"
#include "../Misc/InterfaceFonts.h"
#include <Nuake/Resource/FontAwesome5.h>
#include <Nuake/Resource/ResourceManager.h>
#include <Nuake/FileSystem/FileDialog.h>

#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>


void MaterialEditor::Draw(Ref<Nuake::Material> material)
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
			if (ResourceManager::IsResourceLoaded(material->ID))
			{
				ResourceManager::RegisterResource(material);
			}

			std::string fileData = material->Serialize().dump(4);

			FileSystem::BeginWriteFile(material->Path);
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
		ImGui::BeginTable("##Flags", 3, ImGuiTableFlags_BordersInner | ImGuiTableFlags_SizingStretchProp);
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
			ImGui::TableNextColumn();

			ImGui::TableNextColumn();

			ImGui::Text("Emissive");
			ImGui::TableNextColumn();

			ImGui::DragFloat("##Emissiveness", &material->data.u_Emissive, 0.1f, 1.0f);
			ImGui::TableNextColumn();
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
		AlbedoOpened = ImGui::CollapsingHeader(" ALBEDO", ImGuiTreeNodeFlags_DefaultOpen);
		ImGui::PopStyleVar(2);
	}

	if (AlbedoOpened)
	{
		ImGui::BeginChild("##albedo", TexturePanelSize, true);
		{
			ImTextureID textureID = 0;
			if (material->HasAlbedo())
			{
				auto vkTexture = GPUResources::Get().GetTexture(material->AlbedoImage);
				
				textureID = (ImTextureID)vkTexture->GetImGuiDescriptorSet();
			}

			if (ImGui::ImageButtonEx(ImGui::GetCurrentWindow()->GetID("#image1"), (ImTextureID)textureID, ImVec2(80, 80), ImVec2(0, 1), ImVec2(1, 0), ImVec4(0, 0, 0, 0), ImVec4(1, 1, 1, 1)))
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

			if (ImGui::ImageButtonEx(ImGui::GetCurrentWindow()->GetID("#image3"), (void*)textureID, ImVec2(80, 80), ImVec2(0, 1), ImVec2(1, 0), ImVec4(0, 0, 0, 1), ImVec4(1, 1, 1, 1)))
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

			if (ImGui::ImageButtonEx(ImGui::GetCurrentWindow()->GetID("#image2"), (void*)textureID, ImVec2(80, 80), ImVec2(0, 1), ImVec2(1, 0), ImVec4(1, 1, 1, 1), ImVec4(1, 1, 1, 1)))
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

			ImGui::SameLine();
			ImGui::DragFloat("Value##7", &material->data.u_AOValue, 0.01f, 0.0f, 1.0f);
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

			if (ImGui::ImageButtonEx(ImGui::GetCurrentWindow()->GetID("#image4"), (void*)textureID, ImVec2(80, 80), ImVec2(0, 1), ImVec2(1, 0), ImVec4(0, 0, 0, 1), ImVec4(1, 1, 1, 1)))
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

			if (ImGui::ImageButtonEx(ImGui::GetCurrentWindow()->GetID("#image5"), (void*)textureID, ImVec2(80, 80), ImVec2(0, 1), ImVec2(1, 0), ImVec4(0, 0, 0, 0), ImVec4(1, 1, 1, 1)))
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
			ImGui::SameLine();
			ImGui::DragFloat("Value##6", &material->data.u_RoughnessValue, 0.01f, 0.0f, 1.0f);
		}
		ImGui::EndChild();
	}
}
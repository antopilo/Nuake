#include "SelectionPropertyWidget.h"

#include "Nuake/Scene/Components.h"

#include "Nuake/UI/ImUI.h"
#include "../../../ComponentsPanel/MaterialEditor.h"

#include <Nuake/Resource/SkyResource.h>
#include <Nuake/FileSystem/FileDialog.h>
#include "../../../Misc/ImGuiTextHelper.h"

using namespace Nuake;

SelectionPropertyWidget::SelectionPropertyWidget(EditorContext& inCtx) : 
	IEditorWidget(inCtx)
{
	RegisterComponentDrawer<LightComponent, &LightPanel::Draw>();
	RegisterComponentDrawer<ModelComponent, &MeshPanel::Draw>(&meshPanel);
	//RegisterComponentDrawer<SkinnedModelComponent, &SkinnedMeshPanel::Draw>(&skinnedMeshPanel);
	RegisterComponentDrawer<CameraComponent, &CameraPanel::Draw>();
	RegisterComponentDrawer<MeshColliderComponent, &MeshColliderPanel::Draw>();
	RegisterComponentDrawer<CapsuleColliderComponent, &CapsuleColliderPanel::Draw>();
	RegisterComponentDrawer<NetScriptComponent, &NetScriptPanel::Draw>();
	RegisterComponentDrawer<CylinderColliderComponent, &CylinderColliderPanel::Draw>();
	RegisterComponentDrawer<CharacterControllerComponent, &CharacterControllerPanel::Draw>();
	RegisterComponentDrawer<BoneComponent, &BonePanel::Draw>();
	RegisterComponentDrawer<NavMeshVolumeComponent, &NavMeshVolumePanel::Draw>();

	RegisterTypeDrawer<bool, &SelectionPropertyWidget::DrawFieldTypeBool>(this);
	RegisterTypeDrawer<float, &SelectionPropertyWidget::DrawFieldTypeFloat>(this);
	RegisterTypeDrawer<Vector2, &SelectionPropertyWidget::DrawFieldTypeVector2>(this);
	RegisterTypeDrawer<Vector3, &SelectionPropertyWidget::DrawFieldTypeVector3>(this);
	RegisterTypeDrawer<std::string, &SelectionPropertyWidget::DrawFieldTypeString>(this);
	RegisterTypeDrawer<ResourceFile, &SelectionPropertyWidget::DrawFieldTypeResourceFile>(this);
	RegisterTypeDrawer<DynamicItemList, &SelectionPropertyWidget::DrawFieldTypeDynamicItemList>(this);
}

void SelectionPropertyWidget::Update(float ts)
{
	
}

void SelectionPropertyWidget::Draw()
{
	EditorSelection selection = this->editorContext.GetSelection();
	if (BeginWidgetWindow("Selection Properties"))
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

			if (!selection.File->Exist())
			{
				std::string text = "File is invalid";
				auto windowWidth = ImGui::GetWindowSize().x;
				auto windowHeight = ImGui::GetWindowSize().y;

				auto textWidth = ImGui::CalcTextSize(text.c_str()).x;
				auto textHeight = ImGui::CalcTextSize(text.c_str()).y;
				ImGui::SetCursorPosX((windowWidth - textWidth) * 0.5f);
				ImGui::SetCursorPosY((windowHeight - textHeight) * 0.5f);

				ImGui::TextColored({ 1.f, 0.1f, 0.1f, 1.0f }, text.c_str());
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

void SelectionPropertyWidget::ResolveFile(Ref<Nuake::File> file)
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

	if (currentFile->GetFileType() == FileType::Sky)
	{
		Ref<SkyResource> sky = ResourceLoader::LoadSky(currentFile->GetRelativePath());
		selectedResource = sky;
	}

	if (currentFile->GetFileType() == FileType::Env)
	{
		Ref<Environment> env = ResourceLoader::LoadEnvironment(currentFile->GetRelativePath());
		selectedResource = env;
	}
}

void SelectionPropertyWidget::DrawNone()
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

Nuake::Entity lastEntity;
void SelectionPropertyWidget::DrawEntity(Nuake::Entity entity)
{
    if (!entity.IsValid())
    {
        return;
    }


    if (lastEntity != entity)
    {
        opacity.SetValue(0.0f);
        opacity = 1.0f;
        lastEntity = entity;
    }

    DrawAddComponentMenu(entity);

    transformPanel.Draw(entity);

    entt::registry& registry = entity.GetScene()->m_Registry;
    for (auto&& [componentTypeId, storage] : registry.storage())
    {
        entt::type_info componentType = storage.type();

        entt::entity entityId = static_cast<entt::entity>(entity.GetHandle());
        if (storage.contains(entityId))
        {
            entt::meta_type type = entt::resolve(componentType);
            entt::meta_any component = type.from_void(storage.value(entityId));

            ComponentTypeTrait typeTraits = type.traits<ComponentTypeTrait>();
            // Component not exposed as an inspector panel
            if ((typeTraits & ComponentTypeTrait::InspectorExposed) == ComponentTypeTrait::None)
            {
                continue;
            }

            DrawComponent(entity, component);
        }
    }

    using namespace Nuake;

    float availWidth = ImGui::GetContentRegionAvail().x;
    const float buttonWidth = 200.f;
    float posX = (availWidth / 2.f) - (buttonWidth / 2.f);
    ImGui::SetCursorPosX(posX);

    if (UI::PrimaryButton("Add Component", { buttonWidth, 32 }))
    {
        ImGui::OpenPopup("ComponentPopup");
    }

    if (ImGui::BeginPopup("ComponentPopup"))
    {
        for (auto [fst, component] : entt::resolve())
        {
            std::string componentName = Component::GetName(component);
            if (ImGui::MenuItem(componentName.c_str()))
            {
                entity.AddComponent(component);
            }
        }

        ImGui::EndPopup();
    }

}

void SelectionPropertyWidget::DrawAddComponentMenu(Nuake::Entity entity)
{
    using namespace Nuake;
    if (entity.HasComponent<NameComponent>())
    {
        UIFont* boldIconFont = new UIFont(Fonts::Icons);
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 8.0f);
        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 4.0f);
        ImGui::Text(ICON_FA_BOX);
        delete boldIconFont;

        ImGui::SameLine();
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() - 2.0f);
        ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 4.0f);
        ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0, 0, 0, 0));
        UIFont* boldFont = new UIFont(Fonts::Bold);
        auto& entityName = entity.GetComponent<NameComponent>().Name;

        ImGuiTextSTD("##Name", entityName);
        delete boldFont;

        ImGui::PopStyleColor();
    }
}

void SelectionPropertyWidget::DrawFile(Ref<Nuake::File> file)
{
    using namespace Nuake;
    switch (file->GetFileType())
    {
    case FileType::Material:
    {
        MaterialEditor matEditor;
        matEditor.Draw(std::static_pointer_cast<Material>(selectedResource));
        break;
    }
    case FileType::Project:
    {
        DrawProjectPanel(Nuake::Engine::GetProject());
        break;
    }
    case FileType::Script:
    {
        break;
    }
    case FileType::NetScript:
    {
        DrawNetScriptPanel(file);
        break;
    }
    case FileType::Prefab:
    {
        //Ref<Prefab> prefab = CreateRef<Prefab>(file->GetRelativePath());
        //DrawPrefabPanel(prefab);
        break;
    }
    case FileType::Sky:
    {
        auto sky = std::static_pointer_cast<SkyResource>(selectedResource);
        std::string skyName = sky->Path;
        {
            UIFont boldfont = UIFont(Fonts::SubTitle);
            ImGui::Text(sky->Path.c_str());

        }
        ImGui::SameLine();
        {
            UIFont boldfont = UIFont(Fonts::Icons);
            if (ImGui::Button(ICON_FA_SAVE))
            {
                if (ResourceManager::IsResourceLoaded(sky->ID))
                {
                    ResourceManager::RegisterResource(sky);
                }

                std::string fileData = sky->Serialize().dump(4);

                FileSystem::BeginWriteFile(sky->Path);
                FileSystem::WriteLine(fileData);
                FileSystem::EndWriteFile();
            }
        }

        int textureId = 0;

        // Top
        ImGui::Text("Top");
        if (auto topTexture = sky->GetFaceTexture(SkyFaces::Top);
            !topTexture.empty())
        {
            textureId = TextureManager::Get()->GetTexture(FileSystem::RelativeToAbsolute(topTexture))->GetID();
        }

        if (ImGui::ImageButtonEx(ImGui::GetCurrentWindow()->GetID("#skytexture1"), (void*)textureId, ImVec2(80, 80), ImVec2(0, 1), ImVec2(1, 0), ImVec4(0, 0, 0, 0), ImVec4(1, 1, 1, 1)))
        {
            std::string texture = Nuake::FileDialog::OpenFile("*.png | *.jpg");
            if (!texture.empty())
            {
                sky->SetTextureFace(SkyFaces::Top, FileSystem::AbsoluteToRelative(texture));
            }
        }

        if (ImGui::BeginPopupContextWindow())
        {
            if (ImGui::MenuItem("Clear Texture"))
            {
                sky->SetTextureFace(SkyFaces::Top, "");
            }
            ImGui::EndPopup();
        }

        textureId = 0;

        ImGui::Text("Bottom");
        if (auto bottomTexture = sky->GetFaceTexture(SkyFaces::Bottom);
            !bottomTexture.empty())
        {
            textureId = TextureManager::Get()->GetTexture(FileSystem::RelativeToAbsolute(bottomTexture))->GetID();
        }

        if (ImGui::ImageButtonEx(ImGui::GetCurrentWindow()->GetID("#skytexture2"), (void*)textureId, ImVec2(80, 80), ImVec2(0, 1), ImVec2(1, 0), ImVec4(0, 0, 0, 0), ImVec4(1, 1, 1, 1)))
        {
            std::string texture = FileDialog::OpenFile("*.png | *.jpg");
            if (!texture.empty())
            {
                sky->SetTextureFace(SkyFaces::Bottom, FileSystem::AbsoluteToRelative(texture));
            }
        }

        if (ImGui::BeginPopupContextWindow())
        {
            if (ImGui::MenuItem("Clear Texture"))
            {
                sky->SetTextureFace(SkyFaces::Bottom, "");
            }
            ImGui::EndPopup();
        }

        textureId = 0;

        ImGui::Text("Left");
        if (auto bottomTexture = sky->GetFaceTexture(SkyFaces::Left);
            !bottomTexture.empty())
        {
            textureId = TextureManager::Get()->GetTexture(FileSystem::RelativeToAbsolute(bottomTexture))->GetID();
        }

        if (ImGui::ImageButtonEx(ImGui::GetCurrentWindow()->GetID("#skytexture3"), (void*)textureId, ImVec2(80, 80), ImVec2(0, 1), ImVec2(1, 0), ImVec4(0, 0, 0, 0), ImVec4(1, 1, 1, 1)))
        {
            std::string texture = FileDialog::OpenFile("*.png | *.jpg");
            if (!texture.empty())
            {
                sky->SetTextureFace(SkyFaces::Left, FileSystem::AbsoluteToRelative(texture));
            }
        }

        if (ImGui::BeginPopupContextWindow())
        {
            if (ImGui::MenuItem("Clear Texture"))
            {
                sky->SetTextureFace(SkyFaces::Left, "");
            }
            ImGui::EndPopup();
        }

        textureId = 0;

        ImGui::Text("Right");
        if (auto bottomTexture = sky->GetFaceTexture(SkyFaces::Right);
            !bottomTexture.empty())
        {
            textureId = TextureManager::Get()->GetTexture(FileSystem::RelativeToAbsolute(bottomTexture))->GetID();
        }

        if (ImGui::ImageButtonEx(ImGui::GetCurrentWindow()->GetID("#skytexture4"), (void*)textureId, ImVec2(80, 80), ImVec2(0, 1), ImVec2(1, 0), ImVec4(0, 0, 0, 0), ImVec4(1, 1, 1, 1)))
        {
            std::string texture = FileDialog::OpenFile("*.png | *.jpg");
            if (!texture.empty())
            {
                sky->SetTextureFace(SkyFaces::Right, FileSystem::AbsoluteToRelative(texture));
            }
        }

        if (ImGui::BeginPopupContextWindow())
        {
            if (ImGui::MenuItem("Clear Texture"))
            {
                sky->SetTextureFace(SkyFaces::Right, "");
            }
            ImGui::EndPopup();
        }

        textureId = 0;

        ImGui::Text("Front");
        if (auto bottomTexture = sky->GetFaceTexture(SkyFaces::Front);
            !bottomTexture.empty())
        {
            textureId = TextureManager::Get()->GetTexture(FileSystem::RelativeToAbsolute(bottomTexture))->GetID();
        }

        if (ImGui::ImageButtonEx(ImGui::GetCurrentWindow()->GetID("#skytexture5"), (void*)textureId, ImVec2(80, 80), ImVec2(0, 1), ImVec2(1, 0), ImVec4(0, 0, 0, 0), ImVec4(1, 1, 1, 1)))
        {
            std::string texture = FileDialog::OpenFile("*.png | *.jpg");
            if (!texture.empty())
            {
                sky->SetTextureFace(SkyFaces::Front, FileSystem::AbsoluteToRelative(texture));
            }
        }

        if (ImGui::BeginPopupContextWindow())
        {
            if (ImGui::MenuItem("Clear Texture"))
            {
                sky->SetTextureFace(SkyFaces::Front, "");
            }
            ImGui::EndPopup();
        }

        textureId = 0;

        ImGui::Text("Back");
        if (auto bottomTexture = sky->GetFaceTexture(SkyFaces::Back);
            !bottomTexture.empty())
        {
            textureId = TextureManager::Get()->GetTexture(FileSystem::RelativeToAbsolute(bottomTexture))->GetID();
        }

        if (ImGui::ImageButtonEx(ImGui::GetCurrentWindow()->GetID("#skytexture6"), (void*)textureId, ImVec2(80, 80), ImVec2(0, 1), ImVec2(1, 0), ImVec4(0, 0, 0, 0), ImVec4(1, 1, 1, 1)))
        {
            std::string texture = FileDialog::OpenFile("*.png | *.jpg");
            if (!texture.empty())
            {
                sky->SetTextureFace(SkyFaces::Back, FileSystem::AbsoluteToRelative(texture));
            }
        }

        if (ImGui::BeginPopupContextWindow())
        {
            if (ImGui::MenuItem("Clear Texture"))
            {
                sky->SetTextureFace(SkyFaces::Back, "");
            }
            ImGui::EndPopup();
        }


        break;
    }
    case FileType::Env:
    {
        const Ref<Environment> env = std::static_pointer_cast<Environment>(selectedResource);
        std::string skyName = env->Path;
        {
            UIFont boldfont = UIFont(Fonts::SubTitle);
            ImGui::Text(env->Path.c_str());

        }
        ImGui::SameLine();
        {
            UIFont boldfont = UIFont(Fonts::Icons);
            if (ImGui::Button(ICON_FA_SAVE))
            {
                if (!ResourceManager::IsResourceLoaded(env->ID))
                {
                    ResourceManager::RegisterResource(env);
                }

                std::string fileData = env->Serialize().dump(4);

                FileSystem::BeginWriteFile(env->Path);
                FileSystem::WriteLine(fileData);
                FileSystem::EndWriteFile();
            }
        }

        BEGIN_COLLAPSE_HEADER(SKY);
        if (ImGui::BeginTable("EnvTable", 3, ImGuiTableFlags_BordersInner | ImGuiTableFlags_SizingStretchProp))
        {
            ImGui::TableSetupColumn("name", 0, 0.3f);
            ImGui::TableSetupColumn("set", 0, 0.6f);
            ImGui::TableSetupColumn("reset", 0, 0.1f);

            ImGui::TableNextColumn();
            {
                // Title
                ImGui::Text("Sky Type");
                ImGui::TableNextColumn();

                // Here we create a dropdown for every sky type.
                const char* SkyTypes[] = { "Procedural Sky", "Color" };
                static int currentSkyType = (int)env->CurrentSkyType;
                ImGui::Combo("##SkyType", &currentSkyType, SkyTypes, IM_ARRAYSIZE(SkyTypes));
                env->CurrentSkyType = (SkyType)currentSkyType;
                ImGui::TableNextColumn();

                // Reset button
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1, 1, 1, 0));
                std::string ResetType = ICON_FA_UNDO + std::string("##ResetType");
                if (ImGui::Button(ResetType.c_str())) env->CurrentSkyType = SkyType::ProceduralSky;
                ImGui::PopStyleColor();
            }

            ImGui::TableNextColumn();
            {
                // Title
                ImGui::Text("Gamma");
                ImGui::TableNextColumn();

                // Here we create a dropdown for every sky type.
                ImGui::DragFloat("##gamma", &env->Gamma, 0.001f, 0.0f);
                ImGui::TableNextColumn();

                // Reset button
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1, 1, 1, 0));
                std::string ResetType = ICON_FA_UNDO + std::string("##ResetType");
                if (ImGui::Button(ResetType.c_str())) env->CurrentSkyType = SkyType::ProceduralSky;
                ImGui::PopStyleColor();
            }

            ImGui::TableNextColumn();
            {
                // Title
                ImGui::Text("Exposure");
                ImGui::TableNextColumn();

                // Here we create a dropdown for every sky type.
                ImGui::DragFloat("##exposure", &env->Exposure, 0.001f, 0.0f);
                ImGui::TableNextColumn();

                // Reset button
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1, 1, 1, 0));
                std::string ResetType = ICON_FA_UNDO + std::string("##ResetType");
                if (ImGui::Button(ResetType.c_str())) env->CurrentSkyType = SkyType::ProceduralSky;
                ImGui::PopStyleColor();
            }

            if (env->CurrentSkyType == SkyType::ProceduralSky)
            {
                ImGui::TableNextColumn();

                {   // Sun Intensity
                    ImGui::Text("Sun Intensity");
                    ImGui::TableNextColumn();

                    ImGui::DragFloat("##Sun Intensity", &env->ProceduralSkybox->SunIntensity, 0.1f, 0.0f, 1000.0f);
                    ImGui::TableNextColumn();

                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1, 1, 1, 0));
                    std::string resetSunIntensity = ICON_FA_UNDO + std::string("##ResetSunIntensity");
                    if (ImGui::Button(resetSunIntensity.c_str())) env->ProceduralSkybox->SunIntensity = 100.0f;
                    ImGui::PopStyleColor();
                }

                ImGui::TableNextColumn();
                {   // Sun Direction
                    ImGui::Text("Sun Direction");
                    ImGui::TableNextColumn();

                    Vector3 sunDirection = env->ProceduralSkybox->GetSunDirection();
                    ImGuiHelper::DrawVec3("##Sun Direction", &sunDirection);
                    env->ProceduralSkybox->SunDirection = glm::mix(env->ProceduralSkybox->GetSunDirection(), glm::normalize(sunDirection), 0.1f);
                    ImGui::TableNextColumn();

                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1, 1, 1, 0));
                    std::string resetSunDirection = ICON_FA_UNDO + std::string("##resetSunDirection");
                    if (ImGui::Button(resetSunDirection.c_str())) env->ProceduralSkybox->SunDirection = Vector3(0.20000f, 0.95917f, 0.20000f);
                    ImGui::PopStyleColor();
                }

                ImGui::TableNextColumn();
                {   // Surface Radius
                    ImGui::Text("Surface Radius");
                    ImGui::TableNextColumn();

                    ImGui::DragFloat("##surfaceRadius", &env->ProceduralSkybox->SurfaceRadius, 100.f, 0.0f);
                    ImGui::TableNextColumn();

                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1, 1, 1, 0));
                    std::string resetSurfaceRadius = ICON_FA_UNDO + std::string("##resetSurfaceRadius");
                    if (ImGui::Button(resetSurfaceRadius.c_str())) env->ProceduralSkybox->SurfaceRadius = 6360e3f;
                    ImGui::PopStyleColor();
                }

                ImGui::TableNextColumn();
                {   // Atmosphere Radius
                    ImGui::Text("Atmosphere Radius");
                    ImGui::TableNextColumn();

                    ImGui::DragFloat("##AtmosphereRadius", &env->ProceduralSkybox->AtmosphereRadius, 100.f, 0.0f);
                    ImGui::TableNextColumn();

                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1, 1, 1, 0));
                    std::string resetAtmosphereRadius = ICON_FA_UNDO + std::string("##resetAtmosphereRadius");
                    if (ImGui::Button(resetAtmosphereRadius.c_str())) env->ProceduralSkybox->AtmosphereRadius = 6380e3f;
                    ImGui::PopStyleColor();
                }

                ImGui::TableNextColumn();
                {   // Center point
                    ImGui::Text("Center Point");
                    ImGui::TableNextColumn();

                    ImGuiHelper::DrawVec3("##Center Point", &env->ProceduralSkybox->CenterPoint, 0.0f, 100.0f, 100.0f);
                    ImGui::TableNextColumn();
                    if (env->ProceduralSkybox->CenterPoint.y < -env->ProceduralSkybox->AtmosphereRadius)
                        env->ProceduralSkybox->CenterPoint.y = -env->ProceduralSkybox->AtmosphereRadius + 1.f;
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1, 1, 1, 0));
                    std::string resetCenterPoint = ICON_FA_UNDO + std::string("##resetAtmosphereRadius");
                    if (ImGui::Button(resetCenterPoint.c_str())) env->ProceduralSkybox->CenterPoint = Vector3(0, -env->ProceduralSkybox->AtmosphereRadius, 0);
                    ImGui::PopStyleColor();
                }

                ImGui::TableNextColumn();
                {   // Mie Scattering
                    ImGui::Text("Mie Scattering");
                    ImGui::TableNextColumn();

                    Vector3 mieScattering = env->ProceduralSkybox->MieScattering * 10000.0f;
                    ImGuiHelper::DrawVec3("##Mie Scattering", &mieScattering, 0.0f, 100.0f, 0.01f);
                    if (mieScattering.x < 0) mieScattering.x = 0;
                    if (mieScattering.y < 0) mieScattering.y = 0;
                    if (mieScattering.z < 0) mieScattering.z = 0;
                    env->ProceduralSkybox->MieScattering = mieScattering / 10000.0f;
                    ImGui::TableNextColumn();

                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1, 1, 1, 0));
                    std::string resetMieScattering = ICON_FA_UNDO + std::string("##resetMieScattering");
                    if (ImGui::Button(resetMieScattering.c_str())) env->ProceduralSkybox->MieScattering = Vector3(2e-5f);
                    ImGui::PopStyleColor();
                }

                ImGui::TableNextColumn();
                {   // RayleighScattering
                    ImGui::Text("Rayleigh Scattering");
                    ImGui::TableNextColumn();

                    Vector3 rayleighScattering = env->ProceduralSkybox->RayleighScattering * 10000.0f;
                    ImGuiHelper::DrawVec3("##Ray Scattering", &rayleighScattering, 0.0f, 100.0f, 0.01f);
                    if (rayleighScattering.r < 0) rayleighScattering.r = 0;
                    if (rayleighScattering.g < 0) rayleighScattering.g = 0;
                    if (rayleighScattering.b < 0) rayleighScattering.b = 0;
                    env->ProceduralSkybox->RayleighScattering = rayleighScattering / 10000.0f;
                    ImGui::TableNextColumn();

                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1, 1, 1, 0));
                    std::string resetRayScattering = ICON_FA_UNDO + std::string("##resetRayScattering");
                    if (ImGui::Button(resetRayScattering.c_str())) env->ProceduralSkybox->RayleighScattering = Vector3(58e-7f, 135e-7f, 331e-7f);
                    ImGui::PopStyleColor();
                }
            }

            if (env->CurrentSkyType == SkyType::ClearColor)
            {
                ImGui::TableNextColumn();

                // Title
                ImGui::Text("Clear color");
                ImGui::TableNextColumn();

                // Color picker
                ImGui::ColorEdit4("##clearColor", &env->AmbientColor.r, ImGuiColorEditFlags_NoAlpha);
                ImGui::TableNextColumn();

                // Reset button
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1, 1, 1, 0));
                std::string resetColor = ICON_FA_UNDO + std::string("##ResetColor");
                if (ImGui::Button(resetColor.c_str())) env->AmbientColor = Color(0, 0, 0, 1);
                ImGui::PopStyleColor();
            }

            ImGui::TableNextColumn();
            {
                // Title
                ImGui::Text("Ambient Term");
                ImGui::TableNextColumn();

                // Here we create a dropdown for every sky type.
                ImGui::DragFloat("##AmbientTerm", &env->AmbientTerm, 0.001f, 0.00f, 1.0f);
                ImGui::TableNextColumn();

                // Reset button
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1, 1, 1, 0));
                std::string ResetType = ICON_FA_UNDO + std::string("##ambient");
                if (ImGui::Button(ResetType.c_str())) env->AmbientTerm = 0.25f;
                ImGui::PopStyleColor();
            }

            ImGui::EndTable();
        }
        END_COLLAPSE_HEADER()

            BEGIN_COLLAPSE_HEADER(TAA)
            if (ImGui::BeginTable("EnvTableTAA", 3, ImGuiTableFlags_BordersInner | ImGuiTableFlags_SizingStretchProp))
            {
                ImGui::TableSetupColumn("name", 0, 0.3f);
                ImGui::TableSetupColumn("set", 0, 0.6f);
                ImGui::TableSetupColumn("reset", 0, 0.1f);

                ImGui::TableNextColumn();
                {
                    auto& sceneRenderer = Engine::GetCurrentScene()->m_SceneRenderer;

                    // Title
                    ImGui::Text("TAA Factor");
                    ImGui::TableNextColumn();

                    ImGui::SliderFloat("##TAAFactor", &sceneRenderer->TAAFactor, 0.0f, 1.0f);
                    ImGui::TableNextColumn();

                    // Reset button
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1, 1, 1, 0));
                    std::string resetVolumetric = ICON_FA_UNDO + std::string("##resetTAAFactor");
                    if (ImGui::Button(resetVolumetric.c_str())) sceneRenderer->TAAFactor = 0.6f;
                    ImGui::PopStyleColor();
                }
                ImGui::EndTable();
            }
        END_COLLAPSE_HEADER()
            BEGIN_COLLAPSE_HEADER(BLOOM)
            if (ImGui::BeginTable("BloomTable", 3, ImGuiTableFlags_BordersInner | ImGuiTableFlags_SizingStretchProp))
            {
                ImGui::TableSetupColumn("name", 0, 0.3f);
                ImGui::TableSetupColumn("set", 0, 0.6f);
                ImGui::TableSetupColumn("reset", 0, 0.1f);

                ImGui::TableNextColumn();
                {
                    // Title
                    ImGui::Text("Enabled");
                    ImGui::TableNextColumn();

                    ImGui::Checkbox("##Enabled", &env->BloomEnabled);
                    ImGui::TableNextColumn();

                    // Reset button
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1, 1, 1, 0));
                    std::string resetBloom = ICON_FA_UNDO + std::string("##resetBloom");
                    if (ImGui::Button(resetBloom.c_str())) env->BloomEnabled = false;
                    ImGui::PopStyleColor();
                }

                ImGui::TableNextColumn();
                {
                    // Title
                    ImGui::Text("Threshold");
                    ImGui::TableNextColumn();

                    float threshold = env->mBloom->GetThreshold();
                    ImGui::DragFloat("##Threshold", &threshold, 0.01f, 0.0f, 500.0f);
                    env->mBloom->SetThreshold(threshold);
                    ImGui::TableNextColumn();

                    // Reset button
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1, 1, 1, 0));
                    std::string resetBloomThreshold = ICON_FA_UNDO + std::string("##resetBloomThreshold");
                    if (ImGui::Button(resetBloomThreshold.c_str())) env->mBloom->SetThreshold(2.4f);
                    ImGui::PopStyleColor();
                }

                ImGui::TableNextColumn();
                {
                    // Title
                    ImGui::Text("Quality");
                    ImGui::TableNextColumn();

                    int iteration = env->mBloom->GetIteration();
                    int oldIteration = iteration;
                    ImGui::DragInt("##quality", &iteration, 1.0f, 0, 4);

                    if (oldIteration != iteration)
                    {
                        env->mBloom->SetIteration(iteration);
                    }
                    ImGui::TableNextColumn();

                    // Reset button
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1, 1, 1, 0));
                    std::string resetQuality = ICON_FA_UNDO + std::string("##resetQuality");
                    if (ImGui::Button(resetQuality.c_str())) env->mBloom->SetIteration(3);
                    ImGui::PopStyleColor();
                }

                ImGui::TableNextColumn();
                {
                    // Title
                    ImGui::Text("Lens Dirt");
                    ImGui::TableNextColumn();

                    Ref<Texture> lensTexture = env->mBloom->GetLensDirt();

                    std::string filePath = lensTexture == nullptr ? "None" : lensTexture->GetPath();
                    std::string controlName = filePath + std::string("##") + filePath;
                    ImGui::Button(controlName.c_str(), ImVec2(ImGui::GetContentRegionAvail().x, 0));

                    if (ImGui::BeginDragDropTarget())
                    {
                        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("_Image"))
                        {
                            const char* payloadFilePath = static_cast<char*>(payload->Data);
                            const std::string fullPath = std::string(payloadFilePath, 256);
                            const Ref<Nuake::File> file = FileSystem::GetFile(FileSystem::AbsoluteToRelative(fullPath));
                            env->mBloom->SetLensDirt(TextureManager::Get()->GetTexture(file->GetAbsolutePath()));
                            Engine::GetProject()->IsDirty = true;
                        }
                        ImGui::EndDragDropTarget();
                    }

                    ImGui::TableNextColumn();

                    // Reset button
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1, 1, 1, 0));
                    std::string resetLens = ICON_FA_UNDO + std::string("##resetLens");
                    if (ImGui::Button(resetLens.c_str())) env->mBloom->ClearLensDirt();
                    ImGui::PopStyleColor();
                }

                ImGui::TableNextColumn();
                {
                    // Title
                    ImGui::Text("Lens Dirt Intensity");
                    ImGui::TableNextColumn();

                    float lensDirtIntensity = env->mBloom->GetLensDirtIntensity();
                    ImGui::SliderFloat("##lensDirtIntensity", &lensDirtIntensity, 0.0f, 1.0f);

                    if (lensDirtIntensity != env->mBloom->GetLensDirtIntensity())
                    {
                        env->mBloom->SetLensDirtIntensity(lensDirtIntensity);
                    }

                    ImGui::TableNextColumn();

                    // Reset button
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1, 1, 1, 0));
                    std::string resetLensDirt = ICON_FA_UNDO + std::string("##resetLensDirtIntensity");
                    if (ImGui::Button(resetLensDirt.c_str())) env->mBloom->SetLensDirtIntensity(1.0f);
                    ImGui::PopStyleColor();
                }

                ImGui::EndTable();
            }
        END_COLLAPSE_HEADER()

            BEGIN_COLLAPSE_HEADER(VOLUMETRIC)
            if (ImGui::BeginTable("EnvTable", 3, ImGuiTableFlags_BordersInner | ImGuiTableFlags_SizingStretchProp))
            {
                ImGui::TableSetupColumn("name", 0, 0.3f);
                ImGui::TableSetupColumn("set", 0, 0.6f);
                ImGui::TableSetupColumn("reset", 0, 0.1f);

                ImGui::TableNextColumn();
                {
                    // Title
                    ImGui::Text("Enabled");
                    ImGui::TableNextColumn();

                    ImGui::Checkbox("##VolumetricEnabled", &env->VolumetricEnabled);
                    ImGui::TableNextColumn();

                    // Reset button
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1, 1, 1, 0));
                    std::string resetVolumetric = ICON_FA_UNDO + std::string("##resetVolumetric");
                    if (ImGui::Button(resetVolumetric.c_str())) env->VolumetricEnabled = false;
                    ImGui::PopStyleColor();
                }

                ImGui::TableNextColumn();
                {
                    // Title
                    ImGui::Text("Scattering");
                    ImGui::TableNextColumn();

                    float fogAmount = env->mVolumetric->GetFogAmount();
                    ImGui::DragFloat("##Volumetric Scattering", &fogAmount, .001f, 0.f, 1.0f);
                    env->mVolumetric->SetFogAmount(fogAmount);
                    ImGui::TableNextColumn();

                    // Reset button
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1, 1, 1, 0));
                    std::string resetBloomThreshold = ICON_FA_UNDO + std::string("##resetBloomThreshold");
                    if (ImGui::Button(resetBloomThreshold.c_str())) env->mBloom->SetThreshold(2.4f);
                    ImGui::PopStyleColor();
                }

                ImGui::TableNextColumn();
                {
                    // Title
                    ImGui::Text("Strength");
                    ImGui::TableNextColumn();

                    float fogAmount = env->mVolumetric->GetFogExponant();
                    ImGui::DragFloat("##Volumetric Strength", &fogAmount, .001f, 0.f, 1.0f);
                    env->mVolumetric->SetFogExponant(fogAmount);
                    ImGui::TableNextColumn();

                    // Reset button
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1, 1, 1, 0));
                    std::string resetBloomThreshold = ICON_FA_UNDO + std::string("##resetFogExpo");
                    if (ImGui::Button(resetBloomThreshold.c_str())) env->mBloom->SetThreshold(2.4f);
                    ImGui::PopStyleColor();
                }

                ImGui::TableNextColumn();
                {
                    // Title
                    ImGui::Text("Step count");
                    ImGui::TableNextColumn();

                    int stepCount = env->mVolumetric->GetStepCount();
                    ImGui::DragInt("##Volumetric Step Count", &stepCount, 1.f, 0.0f);
                    env->mVolumetric->SetStepCount(stepCount);

                    ImGui::TableNextColumn();

                    // Reset button
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1, 1, 1, 0));
                    std::string resetQuality = ICON_FA_UNDO + std::string("##resetQuality");
                    if (ImGui::Button(resetQuality.c_str())) env->VolumetricStepCount = 50.f;
                    ImGui::PopStyleColor();
                }

                ImGui::EndTable();
            }
        END_COLLAPSE_HEADER()

            BEGIN_COLLAPSE_HEADER(SSAO)
            if (ImGui::BeginTable("EnvTable", 3, ImGuiTableFlags_BordersInner | ImGuiTableFlags_SizingStretchProp))
            {
                ImGui::TableSetupColumn("name", 0, 0.3f);
                ImGui::TableSetupColumn("set", 0, 0.6f);
                ImGui::TableSetupColumn("reset", 0, 0.1f);

                ImGui::TableNextColumn();
                {
                    // Title
                    ImGui::Text("Enabled");
                    ImGui::TableNextColumn();

                    ImGui::Checkbox("##SSAOEnabled", &env->SSAOEnabled);
                    ImGui::TableNextColumn();

                    // Reset button
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1, 1, 1, 0));
                    std::string resetSSAO = ICON_FA_UNDO + std::string("##resetSSAO");
                    if (ImGui::Button(resetSSAO.c_str())) env->SSAOEnabled = false;
                    ImGui::PopStyleColor();
                }
                ImGui::TableNextColumn();
                {
                    // Title
                    ImGui::Text("Strength");
                    ImGui::TableNextColumn();

                    ImGui::DragFloat("##SSAOStrength", &env->mSSAO->Strength, 0.01f, 0.01f, 10.0f);
                    ImGui::TableNextColumn();

                    // Reset button
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1, 1, 1, 0));
                    std::string resetRadius = ICON_FA_UNDO + std::string("##resetStrength");
                    if (ImGui::Button(resetRadius.c_str())) env->mSSAO->Strength = 2.0f;
                    ImGui::PopStyleColor();
                }

                ImGui::TableNextColumn();
                {
                    // Title
                    ImGui::Text("Radius");
                    ImGui::TableNextColumn();

                    ImGui::DragFloat("##SSAORadius", &env->mSSAO->Radius, 0.01f, 0.0f, 10.0f);
                    ImGui::TableNextColumn();

                    // Reset button
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1, 1, 1, 0));
                    std::string resetRadius = ICON_FA_UNDO + std::string("##resetRadius");
                    if (ImGui::Button(resetRadius.c_str())) env->mSSAO->Radius = 1.0f;
                    ImGui::PopStyleColor();
                }

                ImGui::TableNextColumn();
                {
                    // Title
                    ImGui::Text("Bias");
                    ImGui::TableNextColumn();

                    ImGui::DragFloat("##SSAOBias", &env->mSSAO->Bias, 0.0001f, 0.00001f, 0.5f);
                    ImGui::TableNextColumn();

                    // Reset button
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1, 1, 1, 0));
                    std::string resetBloomThreshold = ICON_FA_UNDO + std::string("##resetSSAOBias");
                    if (ImGui::Button(resetBloomThreshold.c_str())) env->mSSAO->Bias = 0.001f;
                    ImGui::PopStyleColor();
                }

                ImGui::EndTable();
            }
        END_COLLAPSE_HEADER()

            BEGIN_COLLAPSE_HEADER(SSR)
            if (ImGui::BeginTable("EnvTable", 3, ImGuiTableFlags_BordersInner | ImGuiTableFlags_SizingStretchProp))
            {
                ImGui::TableSetupColumn("name", 0, 0.3);
                ImGui::TableSetupColumn("set", 0, 0.6);
                ImGui::TableSetupColumn("reset", 0, 0.1);

                SSR* ssr = env->mSSR.get();
                {
                    ImGui::TableNextColumn();
                    // Title
                    ImGui::Text("Enabled");
                    ImGui::TableNextColumn();

                    ImGui::Checkbox("##SSREnabled", &env->SSREnabled);
                    ImGui::TableNextColumn();

                    // Reset button
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1, 1, 1, 0));
                    std::string resetSSR = ICON_FA_UNDO + std::string("##resetSSR");
                    if (ImGui::Button(resetSSR.c_str())) env->SSREnabled = false;
                    ImGui::PopStyleColor();
                }

                {
                    ImGui::TableNextColumn();
                    // Title
                    ImGui::Text("RayStep");
                    ImGui::TableNextColumn();
                    ImGui::DragFloat("##SSRRS", &ssr->RayStep, 0.01f, 0.0f);
                    ImGui::TableNextColumn();

                    // Reset button
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1, 1, 1, 0));
                    std::string resetVolumetric = ICON_FA_UNDO + std::string("##resetVolumetric");
                    if (ImGui::Button(resetVolumetric.c_str())) env->VolumetricEnabled = false;
                    ImGui::PopStyleColor();
                }

                ImGui::TableNextColumn();
                {
                    // Title
                    ImGui::Text("Iteration Count");
                    ImGui::TableNextColumn();
                    ImGui::DragInt("##SSRRSi", &ssr->IterationCount, 1, 1);
                    ImGui::TableNextColumn();

                    // Reset button
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1, 1, 1, 0));
                    std::string resetVolumetric = ICON_FA_UNDO + std::string("##resetVolumetric");
                    if (ImGui::Button(resetVolumetric.c_str())) env->VolumetricEnabled = false;
                    ImGui::PopStyleColor();
                }

                ImGui::TableNextColumn();
                {
                    // Title
                    ImGui::Text("Distance Bias");
                    ImGui::TableNextColumn();
                    ImGui::DragFloat("##SSRRSid", &ssr->DistanceBias, 0.01f, 0.f);
                    ImGui::TableNextColumn();

                    // Reset button
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1, 1, 1, 0));
                    std::string resetVolumetric = ICON_FA_UNDO + std::string("##resetVolumetric");
                    if (ImGui::Button(resetVolumetric.c_str())) env->VolumetricEnabled = false;
                    ImGui::PopStyleColor();
                }

                ImGui::TableNextColumn();
                {
                    // Title
                    ImGui::Text("Sample Count");
                    ImGui::TableNextColumn();
                    ImGui::DragInt("##SSRRSids", &ssr->SampleCount, 1, 0);
                    ImGui::TableNextColumn();

                    // Reset button
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1, 1, 1, 0));
                    std::string resetVolumetric = ICON_FA_UNDO + std::string("##resetVolumetric");
                    if (ImGui::Button(resetVolumetric.c_str())) env->VolumetricEnabled = false;
                    ImGui::PopStyleColor();
                }

                ImGui::TableNextColumn();
                {
                    // Title
                    ImGui::Text("Sampling");
                    ImGui::TableNextColumn();
                    ImGui::Checkbox("##SSRRSidss", &ssr->SamplingEnabled);
                    ImGui::TableNextColumn();

                    // Reset button
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1, 1, 1, 0));
                    std::string resetVolumetric = ICON_FA_UNDO + std::string("##resetVolumetric");
                    if (ImGui::Button(resetVolumetric.c_str())) env->VolumetricEnabled = false;
                    ImGui::PopStyleColor();
                }


                ImGui::TableNextColumn();
                {
                    // Title
                    ImGui::Text("Expo");
                    ImGui::TableNextColumn();
                    ImGui::Checkbox("##SSRRSidsse", &ssr->ExpoStep);
                    ImGui::TableNextColumn();

                    // Reset button
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1, 1, 1, 0));
                    std::string resetVolumetric = ICON_FA_UNDO + std::string("##resetVolumetric");
                    if (ImGui::Button(resetVolumetric.c_str())) env->VolumetricEnabled = false;
                    ImGui::PopStyleColor();
                }

                ImGui::TableNextColumn();
                {
                    // Title
                    ImGui::Text("Adaptive Steps");
                    ImGui::TableNextColumn();
                    ImGui::Checkbox("##SSRRSidssse", &ssr->AdaptiveStep);
                    ImGui::TableNextColumn();

                    // Reset button
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1, 1, 1, 0));
                    std::string resetVolumetric = ICON_FA_UNDO + std::string("##resetVolumetric");
                    if (ImGui::Button(resetVolumetric.c_str())) env->VolumetricEnabled = false;
                    ImGui::PopStyleColor();
                }

                ImGui::TableNextColumn();
                {
                    // Title
                    ImGui::Text("Binary Search");
                    ImGui::TableNextColumn();
                    ImGui::Checkbox("##SSRRSidsssbe", &ssr->BinarySearch);
                    ImGui::TableNextColumn();

                    // Reset button
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1, 1, 1, 0));
                    std::string resetVolumetric = ICON_FA_UNDO + std::string("##resetVolumetric");
                    if (ImGui::Button(resetVolumetric.c_str())) env->VolumetricEnabled = false;
                    ImGui::PopStyleColor();
                }

                ImGui::TableNextColumn();
                {
                    // Title
                    ImGui::Text("samplingCoefficient");
                    ImGui::TableNextColumn();
                    ImGui::DragFloat("##samplingCoefficient", &ssr->SampleingCoefficient, 0.001f, 0.0f);
                    ImGui::TableNextColumn();

                    // Reset button
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1, 1, 1, 0));
                    std::string resetVolumetric = ICON_FA_UNDO + std::string("##resetVolumetric");
                    if (ImGui::Button(resetVolumetric.c_str())) env->VolumetricEnabled = false;
                    ImGui::PopStyleColor();
                }

                ImGui::EndTable();
            }
        END_COLLAPSE_HEADER()


            BEGIN_COLLAPSE_HEADER(DOF)
            if (ImGui::BeginTable("EnvTable", 3, ImGuiTableFlags_BordersInner | ImGuiTableFlags_SizingStretchProp))
            {
                ImGui::TableSetupColumn("name", 0, 0.3);
                ImGui::TableSetupColumn("set", 0, 0.6);
                ImGui::TableSetupColumn("reset", 0, 0.1);


                {
                    ImGui::TableNextColumn();
                    // Title
                    ImGui::Text("Enabled");
                    ImGui::TableNextColumn();

                    ImGui::Checkbox("##dofEnabled", &env->DOFEnabled);
                    ImGui::TableNextColumn();

                    // Reset button
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1, 1, 1, 0));
                    std::string resetSSR = ICON_FA_UNDO + std::string("##resetrBarrelDistortionEnabled");
                    if (ImGui::Button(resetSSR.c_str())) env->DOFEnabled = false;
                    ImGui::PopStyleColor();
                }


                {
                    ImGui::TableNextColumn();
                    // Title
                    ImGui::Text("Auto focus");
                    ImGui::TableNextColumn();

                    ImGui::Checkbox("##dofautofocus", &env->DOFAutoFocus);
                    ImGui::TableNextColumn();

                    // Reset button
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1, 1, 1, 0));
                    std::string resetDOFFstop = ICON_FA_UNDO + std::string("##resetdofautofocus");
                    if (ImGui::Button(resetDOFFstop.c_str())) env->DOFAutoFocus = false;
                    ImGui::PopStyleColor();
                }

                {
                    ImGui::TableNextColumn();
                    // Title
                    ImGui::Text("Display focus");
                    ImGui::TableNextColumn();

                    ImGui::Checkbox("##dofshowautofocus", &env->DOFShowFocus);
                    ImGui::TableNextColumn();

                    // Reset button
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1, 1, 1, 0));
                    std::string resetDOFShowautofocus = ICON_FA_UNDO + std::string("##resetdofshowautofocus");
                    if (ImGui::Button(resetDOFShowautofocus.c_str())) env->DOFShowFocus = false;
                    ImGui::PopStyleColor();
                }

                {
                    ImGui::TableNextColumn();
                    // Title
                    ImGui::Text("Focus Distance");
                    ImGui::TableNextColumn();

                    ImGui::DragFloat("##doffocalDepth", &env->DOFFocalDepth);
                    ImGui::TableNextColumn();

                    // Reset button
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1, 1, 1, 0));
                    std::string resetdofFocalDepth = ICON_FA_UNDO + std::string("##resetdofFocalDepth");
                    if (ImGui::Button(resetdofFocalDepth.c_str())) env->DOFFocalDepth = 1.0f;
                    ImGui::PopStyleColor();
                }

                {
                    ImGui::TableNextColumn();
                    // Title
                    ImGui::Text("Focus Size");
                    ImGui::TableNextColumn();

                    ImGui::DragFloat("##dofstart", &env->DOFStart);
                    ImGui::TableNextColumn();

                    // Reset button
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1, 1, 1, 0));
                    std::string resetDOFFstop = ICON_FA_UNDO + std::string("##resetdofstartp");
                    if (ImGui::Button(resetDOFFstop.c_str())) env->DOFStart = 1.0f;
                    ImGui::PopStyleColor();
                }

                {
                    ImGui::TableNextColumn();
                    // Title
                    ImGui::Text("Focus Fade");
                    ImGui::TableNextColumn();

                    ImGui::DragFloat("##dofdistance", &env->DOFDist);
                    ImGui::TableNextColumn();

                    // Reset button
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1, 1, 1, 0));
                    std::string resetDOFDist = ICON_FA_UNDO + std::string("##resetDOFDist");
                    if (ImGui::Button(resetDOFDist.c_str())) env->DOFDist = 1.0f;
                    ImGui::PopStyleColor();
                }

                ImGui::EndTable();
            }
        END_COLLAPSE_HEADER()

            BEGIN_COLLAPSE_HEADER(BARREL_DISTORTION)
            if (ImGui::BeginTable("EnvTable", 3, ImGuiTableFlags_BordersInner | ImGuiTableFlags_SizingStretchProp))
            {
                ImGui::TableSetupColumn("name", 0, 0.3f);
                ImGui::TableSetupColumn("set", 0, 0.6f);
                ImGui::TableSetupColumn("reset", 0, 0.1f);

                {
                    ImGui::TableNextColumn();
                    // Title
                    ImGui::Text("Enabled");
                    ImGui::TableNextColumn();

                    ImGui::Checkbox("##BarrelEnabled", &env->BarrelDistortionEnabled);
                    ImGui::TableNextColumn();

                    // Reset button
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1, 1, 1, 0));
                    std::string resetSSR = ICON_FA_UNDO + std::string("##resetrBarrelDistortionEnabled");
                    if (ImGui::Button(resetSSR.c_str())) env->BarrelDistortionEnabled = false;
                    ImGui::PopStyleColor();
                }

                {
                    ImGui::TableNextColumn();
                    // Title
                    ImGui::Text("Distortion");
                    ImGui::TableNextColumn();
                    ImGui::DragFloat("##distortion", &env->BarrelDistortion, 0.01f);
                    ImGui::TableNextColumn();

                    // Reset button
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1, 1, 1, 0));
                    std::string resetVolumetric = ICON_FA_UNDO + std::string("##resetVolumetric");
                    if (ImGui::Button(resetVolumetric.c_str())) env->BarrelDistortion = 0.0f;
                    ImGui::PopStyleColor();
                }

                {
                    ImGui::TableNextColumn();
                    // Title
                    ImGui::Text("Edge Distortion");
                    ImGui::TableNextColumn();
                    ImGui::DragFloat("##edgedistortion", &env->BarrelEdgeDistortion, 0.01f);
                    ImGui::TableNextColumn();

                    // Reset button
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1, 1, 1, 0));
                    std::string resetVolumetric = ICON_FA_UNDO + std::string("##resetVolumetric");
                    if (ImGui::Button(resetVolumetric.c_str())) env->BarrelEdgeDistortion = 0.0f;
                    ImGui::PopStyleColor();
                }

                {
                    ImGui::TableNextColumn();
                    // Title
                    ImGui::Text("Scale Adjustement");
                    ImGui::TableNextColumn();
                    ImGui::DragFloat("##barrelScale", &env->BarrelScale, 0.01f);
                    ImGui::TableNextColumn();

                    // Reset button
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1, 1, 1, 0));
                    std::string resetVolumetric = ICON_FA_UNDO + std::string("##resetVolumetric");
                    if (ImGui::Button(resetVolumetric.c_str())) env->BarrelScale = 1.0f;
                    ImGui::PopStyleColor();
                }
                ImGui::EndTable();
            }
        END_COLLAPSE_HEADER()

            BEGIN_COLLAPSE_HEADER(VIGNETTE)
            if (ImGui::BeginTable("EnvTable", 3, ImGuiTableFlags_BordersInner | ImGuiTableFlags_SizingStretchProp))
            {
                ImGui::TableSetupColumn("name", 0, 0.3);
                ImGui::TableSetupColumn("set", 0, 0.6);
                ImGui::TableSetupColumn("reset", 0, 0.1);


                {
                    ImGui::TableNextColumn();
                    // Title
                    ImGui::Text("Enabled");
                    ImGui::TableNextColumn();

                    ImGui::Checkbox("##VignetteEnabled", &env->VignetteEnabled);
                    ImGui::TableNextColumn();

                    // Reset button
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1, 1, 1, 0));
                    std::string resetSSR = ICON_FA_UNDO + std::string("##resetrBarrelDistortionEnabled");
                    if (ImGui::Button(resetSSR.c_str())) env->VignetteEnabled = false;
                    ImGui::PopStyleColor();
                }

                {
                    ImGui::TableNextColumn();
                    // Title
                    ImGui::Text("Intensity");
                    ImGui::TableNextColumn();
                    ImGui::DragFloat("##vignetteIntensity", &env->VignetteIntensity, 0.1f);
                    ImGui::TableNextColumn();

                    // Reset button
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1, 1, 1, 0));
                    std::string resetVolumetric = ICON_FA_UNDO + std::string("##resetVolumetric");
                    if (ImGui::Button(resetVolumetric.c_str())) env->VignetteIntensity = 0.0f;
                    ImGui::PopStyleColor();
                }

                {
                    ImGui::TableNextColumn();
                    // Title
                    ImGui::Text("Extend");
                    ImGui::TableNextColumn();
                    ImGui::DragFloat("##vignetteExtend", &env->VignetteExtend, 0.01f, 0.0f);
                    ImGui::TableNextColumn();

                    // Reset button
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1, 1, 1, 0));
                    std::string resetVolumetric = ICON_FA_UNDO + std::string("##resetVolumetric");
                    if (ImGui::Button(resetVolumetric.c_str())) env->VignetteExtend = 0.0f;
                    ImGui::PopStyleColor();
                }
                ImGui::EndTable();
            }
        END_COLLAPSE_HEADER()

            BEGIN_COLLAPSE_HEADER(POSTERIZATION)
            if (ImGui::BeginTable("EnvTable", 3, ImGuiTableFlags_BordersInner | ImGuiTableFlags_SizingStretchProp))
            {
                ImGui::TableSetupColumn("name", 0, 0.3);
                ImGui::TableSetupColumn("set", 0, 0.6);
                ImGui::TableSetupColumn("reset", 0, 0.1);

                {
                    ImGui::TableNextColumn();
                    // Title
                    ImGui::Text("Enabled");
                    ImGui::TableNextColumn();

                    ImGui::Checkbox("##PosterizationEnabled", &env->PosterizationEnabled);
                    ImGui::TableNextColumn();

                    // Reset button
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1, 1, 1, 0));
                    std::string resetPosterization = ICON_FA_UNDO + std::string("##resetPosterizationEnabled");
                    if (ImGui::Button(resetPosterization.c_str())) env->PosterizationEnabled = false;
                    ImGui::PopStyleColor();
                }

                {
                    ImGui::TableNextColumn();
                    // Title
                    ImGui::Text("Levels");
                    ImGui::TableNextColumn();
                    ImGui::DragInt("##PosterizationLevels", &env->PosterizationLevels, 1, 4, 25);
                    ImGui::TableNextColumn();

                    // Reset button
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1, 1, 1, 0));
                    std::string resetPosterizationLevels = ICON_FA_UNDO + std::string("##resetPosterizationLevels");
                    if (ImGui::Button(resetPosterizationLevels.c_str())) env->PosterizationLevels = 10;
                    ImGui::PopStyleColor();
                }

                ImGui::EndTable();
            }
        END_COLLAPSE_HEADER()

            BEGIN_COLLAPSE_HEADER(PIXELIZATION)
            if (ImGui::BeginTable("EnvTable", 3, ImGuiTableFlags_BordersInner | ImGuiTableFlags_SizingStretchProp))
            {
                ImGui::TableSetupColumn("name", 0, 0.3);
                ImGui::TableSetupColumn("set", 0, 0.6);
                ImGui::TableSetupColumn("reset", 0, 0.1);

                {
                    ImGui::TableNextColumn();
                    // Title
                    ImGui::Text("Enabled");
                    ImGui::TableNextColumn();

                    ImGui::Checkbox("##PixelizationEnabled", &env->PixelizationEnabled);
                    ImGui::TableNextColumn();

                    // Reset button
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1, 1, 1, 0));
                    std::string resetPixelization = ICON_FA_UNDO + std::string("##resetPixelizationEnabled");
                    if (ImGui::Button(resetPixelization.c_str())) env->PixelizationEnabled = false;
                    ImGui::PopStyleColor();
                }

                {
                    ImGui::TableNextColumn();
                    // Title
                    ImGui::Text("Pixel Size");
                    ImGui::TableNextColumn();
                    ImGui::DragInt("##PixelSize", &env->PixelSize, 1, 1, 25);
                    ImGui::TableNextColumn();

                    // Reset button
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1, 1, 1, 0));
                    std::string resetPixelSize = ICON_FA_UNDO + std::string("##resetPixelSize");
                    if (ImGui::Button(resetPixelSize.c_str())) env->PixelSize = 4;
                    ImGui::PopStyleColor();
                }

                ImGui::EndTable();
            }
        END_COLLAPSE_HEADER()
            break;
    }
    }
}



void SelectionPropertyWidget::DrawResource(Nuake::Resource resource)
{

}

void SelectionPropertyWidget::DrawMaterialPanel(Ref<Nuake::Material> material)
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

            if (ImGui::ImageButtonEx(ImGui::GetCurrentWindow()->GetID("#image1"), (void*)textureID, ImVec2(80, 80), ImVec2(0, 1), ImVec2(1, 0), ImVec4(0, 0, 0, 0), ImVec4(1, 1, 1, 1)))
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
        }
        ImGui::EndChild();
    }
}

void SelectionPropertyWidget::DrawProjectPanel(Ref<Nuake::Project> project)
{
    ImGui::InputText("Project Name", &project->Name);
    ImGui::InputTextMultiline("Project Description", &project->Description);

    if (ImGui::Button("Locate"))
    {
        const std::string& locationPath = Nuake::FileDialog::OpenFile("TrenchBroom (.exe)\0TrenchBroom.exe\0");

        if (!locationPath.empty())
        {
            project->TrenchbroomPath = locationPath;
        }
    }

    ImGui::SameLine();
    ImGui::InputText("Trenchbroom Path", &project->TrenchbroomPath);
}

void SelectionPropertyWidget::DrawNetScriptPanel(Ref<Nuake::File> file)
{
    auto filePath = file->GetRelativePath();
    std::string fileContent = Nuake::FileSystem::ReadFile(filePath);

    ImGui::Text("Content");
    ImGui::SameLine(ImGui::GetWindowWidth() - 90);
    if (ImGui::Button("Open..."))
    {
        Nuake::OS::OpenIn(file->GetAbsolutePath());
    }

    ImGui::Separator();

    ImGui::PushTextWrapPos(ImGui::GetCursorPos().x + ImGui::GetWindowWidth());
    ImGui::Text(fileContent.c_str(), ImGui::GetWindowWidth());

    ImGui::PopTextWrapPos();
}

void SelectionPropertyWidget::DrawComponent(Nuake::Entity& entity, entt::meta_any& component)
{
    // Call into custom component drawer if one is available for this component
    ImGui::PushStyleVar(ImGuiStyleVar_Alpha, opacity);
    const auto componentIdHash = component.type().info().hash();
    if (ComponentTypeDrawers.contains(componentIdHash))
    {
        const auto drawerFn = ComponentTypeDrawers[componentIdHash];
        drawerFn(entity, component);
        ImGui::PopStyleVar();
        return;
    }

    const entt::meta_type componentMeta = component.type();
    const std::string componentName = Component::GetName(componentMeta);

    UIFont* boldFont = new UIFont(Fonts::Bold);
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.f, 0.f));
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.f, 8.f));
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 0.0f);
    bool removed = false;
    bool headerOpened = ImGui::CollapsingHeader(componentName.c_str(), ImGuiTreeNodeFlags_DefaultOpen);

    ImGui::PopStyleVar();
    if (strcmp(componentName.c_str(), "TRANSFORM") != 0 && ImGui::BeginPopupContextItem())
    {
        if (ImGui::Selectable("Remove")) { removed = true; }
        ImGui::EndPopup();
    }

    if (removed)
    {
        auto componentType = component.type();
        entity.RemoveComponent(componentType);
        ImGui::PopStyleVar();
        delete boldFont;
        Engine::GetProject()->IsDirty = true;
    }
    else if (headerOpened)
    {
        delete boldFont;
        ImGui::PopStyleVar();
        ImGui::Indent();

        if (ImGui::BeginTable(componentName.c_str(), 3, ImGuiTableFlags_SizingStretchProp))
        {
            ImGui::TableSetupColumn("name", 0, 0.25f);
            ImGui::TableSetupColumn("set", 0, 0.65f);
            ImGui::TableSetupColumn("reset", 0, 0.1f);

            ImGui::TableNextRow();

            DrawComponentContent(component);

            ImGui::EndTable();
        }
        ImGui::Unindent();
    }
    else
    {
        ImGui::PopStyleVar();
        delete boldFont;
    }
    ImGui::PopStyleVar(2);
}

void SelectionPropertyWidget::DrawComponentContent(entt::meta_any& component)
{
    entt::meta_type componentMeta = component.type();

    // Draw component bound data
    for (auto [fst, dataType] : componentMeta.data())
    {
        const ComponentFieldTrait fieldTraits = dataType.traits<ComponentFieldTrait>();
        // Field marked as internal and thus not exposed to the inspector
        if ((fieldTraits & ComponentFieldTrait::Internal) == ComponentFieldTrait::Internal)
        {
            continue;
        }

        ImGui::TableSetColumnIndex(0);

        // Search for the appropriate drawer for the type
        entt::id_type dataId = dataType.type().id();
        if (FieldTypeDrawers.contains(dataId))
        {
            auto drawerFn = FieldTypeDrawers[dataId];
            drawerFn(dataType, component);
        }
        else
        {
            ImGui::Text("ERR");
        }

        ImGui::TableNextRow();
    }

    // Draw any actions bound to the component
    for (auto [fst, funcMeta] : componentMeta.func())
    {
        const ComponentFuncTrait funcTraits = funcMeta.traits<ComponentFuncTrait>();
        if ((funcTraits & ComponentFuncTrait::Action) == ComponentFuncTrait::Action)
        {
            ImGui::TableSetColumnIndex(0);

            std::string funcDisplayName = "";
            auto prop = funcMeta.prop(HashedName::DisplayName).value();
            if (prop)
            {
                funcDisplayName = std::string(*prop.try_cast<const char*>());
            }

            std::string buttonName = funcDisplayName;
            if (UI::SecondaryButton(buttonName.c_str()))
            {
                entt::meta_any result = funcMeta.invoke(component);
            }

            ImGui::TableNextRow();
        }
    }
}

void SelectionPropertyWidget::DrawFieldTypeFloat(entt::meta_data& field, entt::meta_any& component)
{
    float stepSize = 1.f;
    if (auto prop = field.prop(HashedFieldPropName::FloatStep))
        stepSize = *prop.value().try_cast<float>();

    float min = 0.f;
    if (auto prop = field.prop(HashedFieldPropName::FloatMin))
        min = *prop.value().try_cast<float>();

    float max = 0.f;
    if (auto prop = field.prop(HashedFieldPropName::FloatMax))
        max = *prop.value().try_cast<float>();

    auto propDisplayName = field.prop(HashedName::DisplayName);
    const char* displayName = *propDisplayName.value().try_cast<const char*>();
    if (displayName != nullptr)
    {
        ImGui::Text(displayName);
        ImGui::TableNextColumn();

        auto fieldVal = field.get(component);
        float* floatPtr = fieldVal.try_cast<float>();
        if (floatPtr != nullptr)
        {
            float floatProxy = *floatPtr;
            const std::string controlId = std::string("##") + displayName;
            if (ImGui::DragFloat(controlId.c_str(), &floatProxy, stepSize, min, max))
            {
                field.set(component, floatProxy);
                Engine::GetProject()->IsDirty = true;
            }
        }
        else
        {
            ImGui::Text("ERR");
        }
    }
}

void SelectionPropertyWidget::DrawFieldTypeBool(entt::meta_data& field, entt::meta_any& component)
{
    auto prop = field.prop(HashedName::DisplayName);
    auto propVal = prop.value();
    const char* displayName = *propVal.try_cast<const char*>();

    if (displayName != nullptr)
    {
        ImGui::Text(displayName);
        ImGui::TableNextColumn();

        auto fieldVal = field.get(component);
        bool* boolPtr = fieldVal.try_cast<bool>();
        if (boolPtr != nullptr)
        {
            bool boolProxy = *boolPtr;
            std::string controlId = std::string("##") + displayName;
            if (ImGui::Checkbox(controlId.c_str(), &boolProxy))
            {
                field.set(component, boolProxy);
                Engine::GetProject()->IsDirty = true;
            }
        }
        else
        {
            ImGui::Text("ERR");
        }
    }
}

void SelectionPropertyWidget::DrawFieldTypeVector3(entt::meta_data& field, entt::meta_any& component)
{
    auto prop = field.prop(HashedName::DisplayName);
    auto propVal = prop.value();
    const char* displayName = *propVal.try_cast<const char*>();

    if (displayName != nullptr)
    {
        ImGui::Text(displayName);
        ImGui::TableNextColumn();

        auto fieldVal = field.get(component);
        Vector3* vec3Ptr = fieldVal.try_cast<Vector3>();
        std::string controlId = std::string("##") + displayName;
        ImGui::PushID(controlId.c_str());

        if (ImGuiHelper::DrawVec3(controlId, vec3Ptr, 0.5f, 100.0, 0.01f))
        {
            field.set(component, *vec3Ptr);
            Engine::GetProject()->IsDirty = true;
        }

        ImGui::PopID();
    }
}

void SelectionPropertyWidget::DrawFieldTypeVector2(entt::meta_data& field, entt::meta_any& component)
{
    auto prop = field.prop(HashedName::DisplayName);
    auto propVal = prop.value();
    const char* displayName = *propVal.try_cast<const char*>();

    if (displayName != nullptr)
    {
        ImGui::Text(displayName);
        ImGui::TableNextColumn();

        auto fieldVal = field.get(component);
        Vector2* vec2Ptr = fieldVal.try_cast<Vector2>();
        std::string controlId = std::string("##") + displayName;
        ImGui::PushID(controlId.c_str());

        if (ImGuiHelper::DrawVec2(controlId, vec2Ptr, 0.5f, 100.0, 0.01f))
        {
            field.set(component, *vec2Ptr);
            Engine::GetProject()->IsDirty = true;
        }

        ImGui::PopID();
    }
}

void SelectionPropertyWidget::DrawFieldTypeString(entt::meta_data& field, entt::meta_any& component)
{
    auto prop = field.prop(HashedName::DisplayName);
    auto propVal = prop.value();
    const char* displayName = *propVal.try_cast<const char*>();

    if (displayName != nullptr)
    {
        ImGui::Text(displayName);
        ImGui::TableNextColumn();

        auto fieldVal = field.get(component);
        std::string* fieldValPtr = fieldVal.try_cast<std::string>();
        if (fieldValPtr != nullptr)
        {
            std::string fieldValProxy = *fieldValPtr;
            std::string controlId = std::string("##") + displayName;
            ImGui::InputText(controlId.c_str(), &fieldValProxy);

            //if (fieldValProxy != fieldVal)
            //{
            //	Engine::GetProject()->IsDirty = true;
            //}
        }
        else
        {
            ImGui::Text("ERR");
        }
    }
}

void SelectionPropertyWidget::DrawFieldTypeResourceFile(entt::meta_data& field, entt::meta_any& component)
{
    const char* resourceRestrictedType = nullptr;
    if (auto prop = field.prop(HashedFieldPropName::ResourceFileType))
        resourceRestrictedType = *prop.value().try_cast<const char*>();

    auto propDisplayName = field.prop(HashedName::DisplayName);
    const char* displayName = *propDisplayName.value().try_cast<const char*>();
    if (displayName != nullptr)
    {
        ImGui::Text(displayName);
        ImGui::TableNextColumn();

        auto fieldVal = field.get(component);
        auto fieldValPtr = fieldVal.try_cast<ResourceFile>();
        if (fieldValPtr != nullptr)
        {
            ImGui::SetNextItemAllowOverlap();

            auto fieldValProxy = *fieldValPtr;
            std::string filePath = fieldValProxy.file == nullptr ? "" : fieldValProxy.file->GetRelativePath();
            std::string controlName = filePath + std::string("##") + displayName;
            ImGui::Button(controlName.c_str(), ImVec2(ImGui::GetContentRegionAvail().x, 0));

            if (ImGui::BeginDragDropTarget())
            {
                if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(resourceRestrictedType))
                {
                    const char* payloadFilePath = static_cast<char*>(payload->Data);
                    const std::string fullPath = std::string(payloadFilePath, 256);
                    const Ref<Nuake::File> file = FileSystem::GetFile(FileSystem::AbsoluteToRelative(fullPath));
                    field.set(component, ResourceFile{ file });
                    Engine::GetProject()->IsDirty = true;
                }
                ImGui::EndDragDropTarget();
            }

            ImGui::SameLine();

            bool showShortcutBtn = !filePath.empty();
            const int shortcutBtnWidth = 30;
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() - shortcutBtnWidth - ImGui::GetStyle().ItemSpacing.x);
            if (ImGui::Button(">", ImVec2(shortcutBtnWidth, 0)))
            {
                if (FileSystem::FileExists(filePath))
                {
                    Ref<Nuake::File> file = FileSystem::GetFile(filePath);
                    if (file->GetFileType() == FileType::Map)
                    {
                        OS::OpenTrenchbroomMap(file->GetAbsolutePath());
                    }
                    else
                    {
                        editorContext.SetSelection(FileSystem::GetFile(filePath));
                    }
                }
            }
        }
        else
        {
            ImGui::Text("ERR");
        }
    }
}

void SelectionPropertyWidget::DrawFieldTypeDynamicItemList(entt::meta_data& field, entt::meta_any& component)
{
    auto propDisplayName = field.prop(HashedName::DisplayName);
    const char* displayName = *propDisplayName.value().try_cast<const char*>();
    if (displayName != nullptr)
    {
        ImGui::Text(displayName);
        ImGui::TableNextColumn();

        auto fieldVal = field.get(component);
        auto fieldValPtr = fieldVal.try_cast<DynamicItemList>();
        if (fieldValPtr == nullptr)
        {
            ImGui::Text("ERR");
        }

        const auto& items = fieldValPtr->items;
        const int index = fieldValPtr->index;

        // Check first to see if we are within the bounds
        std::string selectedStr = "";
        if (index >= 0 || index < items.size())
        {
            selectedStr = items[index];
        }

        std::string controlName = std::string("##") + displayName;
        if (ImGui::BeginCombo(controlName.c_str(), selectedStr.c_str()))
        {
            for (int i = 0; i < items.size(); i++)
            {
                bool isSelected = (index == i);
                std::string name = items[i];

                if (name.empty())
                {
                    name = "Empty";
                }

                if (ImGui::Selectable(name.c_str(), isSelected))
                {
                    field.set(component, i);
                }

                if (isSelected)
                {
                    ImGui::SetItemDefaultFocus();
                }
            }
            ImGui::EndCombo();
        }
    }
}


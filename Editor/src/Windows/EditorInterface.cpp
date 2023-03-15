#include <map>
#include <algorithm>

#include "ProjectInterface.h"
#include "EditorInterface.h"

#include "Engine.h"
#include "src/Core/Logger.h"

#include "src/Vendors/imgui/imgui.h"
#include "src/Vendors/imgui/ImGuizmo.h"
#include "src/Scene/Entities/ImGuiHelper.h"

#include "src/Rendering/Textures/Texture.h"
#include "src/Rendering/Textures/MaterialManager.h"

#include "src/Core/Physics/PhysicsManager.h"

#include "src/Vendors/glm/gtc/type_ptr.hpp"
#include "src/Vendors/glm/gtx/matrix_decompose.hpp"
#include "src/Resource/FontAwesome5.h"

#include "Dependencies/GLEW/include/GL/glew.h"

#include "src/Scene/Scene.h"
#include "src/Scene/Components/Components.h"
#include "src/Scene/Components/BoxCollider.h"
#include "src/Scene/Systems/QuakeMapBuilder.h"
#include "src/Scene/Components/LightComponent.h"
#include "../UIComponents/Viewport.h"
#include <src/Resource/Prefab.h>
#include <src/Scene/Components/PrefabComponent.h>
#include <src/Rendering/Shaders/ShaderManager.h>
#include "src/Rendering/Renderer.h"
#include <src/Scene/Components/InterfaceComponent.h>
#include "src/Core/Input.h"

#include "../Actions/EditorSelection.h"
#include "FileSystemUI.h"

#include "../Misc/InterfaceFonts.h"

#include "WelcomeWindow.h"
#include "src/Rendering/SceneRenderer.h"
#include <dependencies/glfw/include/GLFW/glfw3.h>
#include <src/Rendering/Buffers/Framebuffer.h>

namespace Nuake {
    Ref<UI::UserInterface> userInterface;
    ImFont* normalFont;
    ImFont* boldFont;
    ImFont* EditorInterface::bigIconFont;

    EditorInterface::EditorInterface()
    {
        filesystem = new FileSystemUI(this);
        _WelcomeWindow = new WelcomeWindow(this);

        m_EntitySelectionFramebuffer = CreateRef<FrameBuffer>(false, Vector2(1280, 720));
    }

    void EditorInterface::Init()
    {
        ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
        static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_AutoHideTabBar;
        ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->GetWorkPos());
        ImGui::SetNextWindowSize(viewport->GetWorkSize());
        ImGui::SetNextWindowViewport(viewport->ID);

        ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
        ImGui::DockSpaceOverViewport(viewport, dockspace_flags);
        //this->filesystem = FileSystemUI();
    }


    ImVec2 LastSize = ImVec2();
    void EditorInterface::DrawViewport()
    {
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
        std::string name = ICON_FA_GAMEPAD + std::string(" Scene");
        if (ImGui::Begin(name.c_str()))
        {
            ImGui::PopStyleVar();
            Overlay();
            ImGuizmo::BeginFrame();

            ImGuizmo::SetOrthographic(false);
            ImVec2 regionAvail = ImGui::GetContentRegionAvail();
            Vector2 viewportPanelSize = glm::vec2(regionAvail.x, regionAvail.y);

            Ref<FrameBuffer> framebuffer = Engine::GetCurrentWindow()->GetFrameBuffer();
            if (framebuffer->GetSize() != viewportPanelSize)
                framebuffer->QueueResize(viewportPanelSize);

            Ref<Texture> texture = framebuffer->GetTexture();
            ImGui::Image((void*)texture->GetID(), regionAvail, ImVec2(0, 1), ImVec2(1, 0));

            const Vector2& mousePos = Input::GetMousePosition();
            const ImVec2& windowPos = ImGui::GetWindowPos();
            const ImVec2& windowSize = ImGui::GetWindowSize();
            const bool isInsideWidth = mousePos.x > windowPos.x && mousePos.x < windowPos.x + windowSize.x;
            const bool isInsideHeight = mousePos.y > windowPos.y && mousePos.y < windowPos.y + windowSize.y;
            m_IsHoveringViewport = isInsideWidth && isInsideHeight;

            ImGuizmo::SetDrawlist();
            ImGuizmo::AllowAxisFlip(false);
            ImGuizmo::SetRect(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y, ImGui::GetWindowWidth(), ImGui::GetWindowHeight());

            if (m_DrawGrid && !Engine::IsPlayMode)
            {
                ImGuizmo::DrawGrid(glm::value_ptr(Engine::GetCurrentScene()->GetCurrentCamera()->GetTransform()),
                    glm::value_ptr(Engine::GetCurrentScene()->GetCurrentCamera()->GetPerspective()),
                    glm::value_ptr(glm::identity<glm::mat4>()), 100.f);
            }

            if (Selection.Type == EditorSelectionType::Entity && !Engine::IsPlayMode)
            {
                TransformComponent& tc = Selection.Entity.GetComponent<TransformComponent>();
                ParentComponent& parent = Selection.Entity.GetComponent<ParentComponent>();
                Matrix4 transform = tc.GetGlobalTransform();
                
                auto editorCam = Engine::GetCurrentScene()->GetCurrentCamera();
                Matrix4 cameraView = editorCam->GetTransform();
                Matrix4 cameraProjection = editorCam->GetPerspective();

                ImGuizmo::Manipulate(
                    glm::value_ptr(cameraView),
                    glm::value_ptr(cameraProjection),
                    CurrentOperation, CurrentMode, 
                    glm::value_ptr(transform)
                );
                Matrix4 oldTransform = transform;
                if (ImGuizmo::IsUsing())
                {
                    Vector3 globalPos = Vector3();
                    Entity currentParent = Selection.Entity;
                    if (parent.HasParent)
                    {
                        Matrix4 inverseParent = glm::inverse(parent.Parent.GetComponent<TransformComponent>().GetGlobalTransform());
                        oldTransform *= inverseParent;
                    }

                    Vector3 scale = Vector3();
                    Quat rotation = Quat();
                    Vector3 pos = Vector3();
                    Vector3 skew = Vector3();
                    Vector4 pesp = Vector4();
                    glm::decompose(oldTransform, scale, rotation, pos, skew, pesp);
                    
                    tc.Translation = pos;
                    tc.Rotation = rotation;
                    tc.Scale = scale;
                    tc.LocalTransform = oldTransform;
                    
                    Vector3 gscale = Vector3();
                    Quat grotation = Quat();
                    Vector3 gpos = Vector3();
                    Vector3 gskew = Vector3();
                    Vector4 gpesp = Vector4();
                    glm::decompose(transform, gscale, grotation, gpos, skew, pesp);
                    
                    tc.SetGlobalPosition(gpos);
                    tc.SetGlobalRotation(grotation);
                    tc.SetGlobalScale(gscale);
                    tc.SetGlobalTransform(transform);
                }
            }
        }
        else
        {
            ImGui::PopStyleVar();
        }
        ImGui::End();
    }

    static int selected = 0;
    Entity QueueDeletion;
    void EditorInterface::DrawEntityTree(Entity e)
    {
        ImGuiTreeNodeFlags base_flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_FramePadding | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_SpanAvailWidth;

		NameComponent& nameComponent = e.GetComponent<NameComponent>();
		std::string name = nameComponent.Name;
        ParentComponent& parent = e.GetComponent<ParentComponent>();

        if (Selection.Type == EditorSelectionType::Entity && Selection.Entity == e)
            base_flags |= ImGuiTreeNodeFlags_Selected;

        ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2{ 0.0f, 0.0f });
        {
            ImGui::TableNextColumn();

            bool& isVisible = e.GetComponent<VisibilityComponent>().Visible;
            char* visibilityIcon = isVisible ? ICON_FA_EYE : ICON_FA_EYE_SLASH;


            ImGui::PushStyleColor(ImGuiCol_Button, { 0, 0, 0, 0 });
            if (ImGui::Button(visibilityIcon, { 40, 40 }))
            {
                isVisible = !isVisible;
            }
            ImGui::PopStyleColor();
        }

        ImGui::PopStyleVar();

        ImGui::TableNextColumn();
        
        // Write in normal font.
        ImGui::PushFont(normalFont);

        // If has no childrens draw tree node leaf
        if (parent.Children.size() <= 0)
        {
            base_flags |= ImGuiTreeNodeFlags_Leaf;
        }

        if (nameComponent.IsPrefab && e.HasComponent<PrefabComponent>())
        {
            ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0, 255, 0, 255));
        }

        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(8, 8));
        bool open = ImGui::TreeNodeEx(name.c_str(), base_flags);
        ImGui::PopStyleVar();

		if(nameComponent.IsPrefab && e.HasComponent<PrefabComponent>())
			ImGui::PopStyleColor();
		else
			if (ImGui::BeginDragDropSource())
			{
				ImGui::SetDragDropPayload("ENTITY", (void*)&e, sizeof(Entity));
				ImGui::Text(name.c_str());
				ImGui::EndDragDropSource();
			}

        if (ImGui::BeginDragDropTarget())
        {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ENTITY"))
            {
                Entity payload_entity = *(const Entity*)payload->Data;

                // Check if entity is already parent.
                ParentComponent& parentPayload = payload_entity.GetComponent<ParentComponent>();
                if (!EntityContainsItself(payload_entity, e) && parentPayload.Parent != e && std::count(parent.Children.begin(), parent.Children.end(), payload_entity) == 0)
                {
                    if (parentPayload.HasParent)
                    {
                        // Erase remove idiom.
                        ParentComponent& childOfParent = parentPayload.Parent.GetComponent<ParentComponent>();
                        childOfParent.Children.erase(std::remove(childOfParent.Children.begin(), childOfParent.Children.end(), payload_entity), childOfParent.Children.end());
                    }

                    parentPayload.Parent = e;
                    parentPayload.HasParent = true;
                    parent.Children.push_back(payload_entity);
                }
            }
            ImGui::EndDragDropTarget();
        }

        if (ImGui::IsItemClicked())
            Selection = EditorSelection(e);

        if (ImGui::BeginPopupContextItem())
        {
            Selection = EditorSelection(e);

            Entity entity = Selection.Entity;
            if (entity.HasComponent<CameraComponent>())
            {
                // Moves the editor camera to the camera position and direction.
                if (ImGui::Selectable("Focus camera"))
                {
                    Ref<EditorCamera> editorCam = Engine::GetCurrentScene()->m_EditorCamera;
                    editorCam->Translation = entity.GetComponent<TransformComponent>().GetGlobalPosition();
                    Vector3 camDirection = entity.GetComponent<CameraComponent>().CameraInstance->GetDirection();
                    editorCam->SetDirection(camDirection);
                }
                ImGui::Separator();
            }

            if (ImGui::Selectable("Remove"))
            {
                QueueDeletion = e;
            }

            if (entity.GetComponent<ParentComponent>().HasParent && ImGui::Selectable("Move to root"))
            {
                auto& parentComp = Selection.Entity.GetComponent<ParentComponent>();
                if (parentComp.HasParent)
                {
                    auto& parentParentComp = parentComp.Parent.GetComponent<ParentComponent>();
                    parentParentComp.RemoveChildren(entity);
                    parentComp.HasParent = false;
                }
            }

            if (ImGui::Selectable("Save entity as a new prefab"))
            {
                Ref<Prefab> newPrefab = Prefab::CreatePrefabFromEntity(Selection.Entity);
                std::string savePath = FileDialog::SaveFile("*.prefab");
                newPrefab->SaveAs(savePath);
                Selection.Entity.AddComponent<PrefabComponent>().PrefabInstance = newPrefab;
            }
            ImGui::EndPopup();
        }
        
        if (open)
        {
            // Caching list to prevent deletion while iterating.
            std::vector<Entity> childrens = parent.Children;
            for (auto& c : childrens)
                DrawEntityTree(c);

            ImGui::TreePop();
        }
         
        ImGui::PopFont();
    }

#define BEGIN_COLLAPSE_HEADER(names) \
    UIFont* names##_boldFont = new UIFont(Fonts::Bold); \
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.f, 0.f)); \
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.f, 8.f)); \
    bool names##_Opened = ImGui::CollapsingHeader(#names); \
    ImGui::PopStyleVar(); \
    delete names##_boldFont; \
    if (names##_Opened) { \

#define END_COLLAPSE_HEADER() \
} \
    ImGui::PopStyleVar(); \

    void EditorInterface::DrawSceneTree()
    {
        Ref<Scene> scene = Engine::GetCurrentScene();
          
        if (!scene)
            return;
        const Ref<Environment> env = Engine::GetCurrentScene()->GetEnvironment();

        if (ImGui::Begin("Environnement"))
        {
            BEGIN_COLLAPSE_HEADER(SKY);
                if (ImGui::BeginTable("EnvTable", 3, ImGuiTableFlags_BordersInner))
                {
                    ImGui::TableSetupColumn("name", 0, 0.3);
                    ImGui::TableSetupColumn("set", 0, 0.6);
                    ImGui::TableSetupColumn("reset", 0, 0.1);

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

                    ImGui::EndTable();
                }
                
                if (env->CurrentSkyType == SkyType::ProceduralSky)
                {
                    BEGIN_COLLAPSE_HEADER(SUN)
                        if (ImGui::BeginTable("SunSettingsTable", 3, ImGuiTableFlags_BordersInner))
                        {
                            ImGui::TableSetupColumn("name", 0, 0.3);
                            ImGui::TableSetupColumn("set", 0, 0.6);
                            ImGui::TableSetupColumn("reset", 0, 0.1);
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
                                env->ProceduralSkybox->SunDirection = glm::normalize(sunDirection);
                                ImGui::TableNextColumn();

                                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1, 1, 1, 0));
                                std::string resetSunDirection = ICON_FA_UNDO + std::string("##resetSunDirection");
                                if (ImGui::Button(resetSunDirection.c_str())) env->ProceduralSkybox->SunDirection = Vector3(0.20000f, 0.95917f, 0.20000f);
                                ImGui::PopStyleColor();
                            }

                            ImGui::EndTable();
                        }
                    END_COLLAPSE_HEADER()

                    BEGIN_COLLAPSE_HEADER(ATMOSPHERE)
                        if (ImGui::BeginTable("AtmosphereSettingsTable", 3, ImGuiTableFlags_BordersInner))
                        {
                            ImGui::TableSetupColumn("name", 0, 0.3);
                            ImGui::TableSetupColumn("set", 0, 0.6);
                            ImGui::TableSetupColumn("reset", 0, 0.1);

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

                            ImGui::EndTable();
                        }
                    END_COLLAPSE_HEADER()
                }
            END_COLLAPSE_HEADER()

            BEGIN_COLLAPSE_HEADER(POSTFX)
                if (ImGui::BeginTable("EnvTable", 3, ImGuiTableFlags_BordersInner))
                {
                    ImGui::TableSetupColumn("name", 0, 0.3);
                    ImGui::TableSetupColumn("set", 0, 0.6);
                    ImGui::TableSetupColumn("reset", 0, 0.1);

                    ImGui::TableNextColumn();
                    {
                        // Title
                        ImGui::Text("Bloom");
                        ImGui::TableNextColumn();

                        ImGui::Checkbox("##Enabled", &env->BloomEnabled);
                        ImGui::TableNextColumn();

                        // Reset button
                        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1, 1, 1, 0));
                        std::string resetBloom = ICON_FA_UNDO + std::string("##resetBloom");
                        if (ImGui::Button(resetBloom.c_str())) env->BloomEnabled = false;
                        ImGui::PopStyleColor();
                    }

                    if (env->SSAOEnabled)
                    {
                        ImGui::TableNextColumn();
                        {
                            // Title
                            ImGui::Text("SSAO Strength");
                            ImGui::TableNextColumn();

                            ImGui::DragFloat("##SSAOStrength", &env->mSSAO->Strength, 0.1f, 0.0f, 10.0f);
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
                            ImGui::Text("SSAO Radius");
                            ImGui::TableNextColumn();

                            ImGui::DragFloat("##SSAORadius", &env->mSSAO->Radius, 0.01f, 0.0f, 1.0f);
                            ImGui::TableNextColumn();

                            // Reset button
                            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1, 1, 1, 0));
                            std::string resetRadius = ICON_FA_UNDO + std::string("##resetRadius");
                            if (ImGui::Button(resetRadius.c_str())) env->mSSAO->Radius = 0.5f;
                            ImGui::PopStyleColor();
                        }

                        ImGui::TableNextColumn();
                        {
                            // Title
                            ImGui::Text("SSAO Bias");
                            ImGui::TableNextColumn();

                            ImGui::DragFloat("##SSAOBias", &env->mSSAO->Bias, 0.0001f, 0.0f, 0.5f);
                            ImGui::TableNextColumn();

                            // Reset button
                            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1, 1, 1, 0));
                            std::string resetBloomThreshold = ICON_FA_UNDO + std::string("##resetSSAOBias");
                            if (ImGui::Button(resetBloomThreshold.c_str())) env->mSSAO->Bias =  0.025f;
                            ImGui::PopStyleColor();
                        }

                        ImGui::TableNextColumn();
                        {
                            // Title
                            ImGui::Text("SSAO Area");
                            ImGui::TableNextColumn();

                            ImGui::DragFloat("##SSAOArea", &env->mSSAO->Area, 0.0001f, 0.0f, 0.5f);
                            ImGui::TableNextColumn();

                            // Reset button
                            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1, 1, 1, 0));
                            std::string resetBloomThreshold = ICON_FA_UNDO + std::string("##resetSSAOArea");
                            if (ImGui::Button(resetBloomThreshold.c_str())) env->mSSAO->Area = 0.0075f;
                            ImGui::PopStyleColor();
                        }

                        ImGui::TableNextColumn();
                        {
                            // Title
                            ImGui::Text("SSAO Falloff");
                            ImGui::TableNextColumn();

                            ImGui::DragFloat("##SSAOFalloff", &env->mSSAO->Falloff, 0.0001f, 0.0f, 0.5f);
                            ImGui::TableNextColumn();

                            // Reset button
                            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1, 1, 1, 0));
                            std::string resetBloomThreshold = ICON_FA_UNDO + std::string("##resetSSAOFalloff");
                            if (ImGui::Button(resetBloomThreshold.c_str())) env->mSSAO->Falloff = 0.0022f;
                            ImGui::PopStyleColor();
                        }
                    }

                    if (env->BloomEnabled)
                    {
                        ImGui::TableNextColumn();
                        {
                            // Title
                            ImGui::Text("Bloom Threshold");
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
                            ImGui::Text("Bloom Quality");
                            ImGui::TableNextColumn();

                            int iteration = env->mBloom->GetIteration();
                            ImGui::DragInt("##quality", &iteration, 1.0f, 0, 4);
                            env->mBloom->SetIteration(iteration);
                            ImGui::TableNextColumn();

                            // Reset button
                            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1, 1, 1, 0));
                            std::string resetQuality = ICON_FA_UNDO + std::string("##resetQuality");
                            if (ImGui::Button(resetQuality.c_str())) env->mBloom->SetIteration(3);
                            ImGui::PopStyleColor();
                        }
                    }

                    ImGui::TableNextColumn();
                    {
                        // Title
                        ImGui::Text("Volumetric");
                        ImGui::TableNextColumn();

                        ImGui::Checkbox("##VolumetricEnabled", &env->VolumetricEnabled);
                        ImGui::TableNextColumn();

                        // Reset button
                        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1, 1, 1, 0));
                        std::string resetVolumetric = ICON_FA_UNDO + std::string("##resetVolumetric");
                        if (ImGui::Button(resetVolumetric.c_str())) env->VolumetricEnabled = false;
                        ImGui::PopStyleColor();
                    }
                    
                    if (env->VolumetricEnabled)
                    {
                        ImGui::TableNextColumn();
                        {
                            // Title
                            ImGui::Text("Volumetric Scattering");
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
                    }

                    SSR* ssr = scene->mSceneRenderer->mSSR.get();
                    ImGui::TableNextColumn();
                    {
                        // Title
                        ImGui::Text("SSR RayStep");
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
                        ImGui::Text("iterationCount");
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
                        ImGui::Text("distanceBias");
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
                        ImGui::Text("sampleCount");
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
                        ImGui::Text("Sampling Enabled");
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
                        ImGui::Text("Binary Searcg");
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
                        ImGui::Text("Debug");
                        ImGui::TableNextColumn();
                        ImGui::Checkbox("##SSRRSidsssbed", &ssr->DebugDraw);
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

                    if (env->VolumetricEnabled)
                    {
                        ImGui::TableNextColumn();
                        {
                            // Title
                            ImGui::Text("Volumetric Scattering");
                            ImGui::TableNextColumn();

                            ImGui::DragFloat("##Volumetric Scattering", &env->VolumetricFog, .01f, 0.0f, 1.0f);
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
                            ImGui::Text("Step count");
                            ImGui::TableNextColumn();

                            ImGui::DragFloat("##Volumetric Step Count", &env->VolumetricStepCount, 1.f, 0.0f);
                            ImGui::TableNextColumn();

                            // Reset button
                            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1, 1, 1, 0));
                            std::string resetQuality = ICON_FA_UNDO + std::string("##resetQuality");
                            if (ImGui::Button(resetQuality.c_str())) env->VolumetricStepCount = 50.f;
                            ImGui::PopStyleColor();
                        }
                    }

                }
                ImGui::EndTable();
            END_COLLAPSE_HEADER()

            if (ImGui::CollapsingHeader("Camera", ImGuiTreeNodeFlags_DefaultOpen))
            {
                ImGui::DragFloat("Exposure", &Engine::GetCurrentScene()->m_EditorCamera->Exposure, .01f, 0.0f, 5.0f);
            }
        }
        ImGui::End();

        std::string title = ICON_FA_TREE + std::string(" Hierarchy");
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
        if (ImGui::Begin(title.c_str()))
        {
            // Buttons to add and remove entity.
            if(ImGui::BeginChild("Buttons", ImVec2(ImGui::GetContentRegionAvailWidth(), 30), false))
            {
                // Add entity.
                if (ImGui::Button(ICON_FA_PLUS))
                    Engine::GetCurrentScene()->CreateEntity("Entity");

               //// Remove Entity
               //if (ImGui::Button("Remove"))
               //{
               //    scene->DestroyEntity(m_SelectedEntity);
               //
               //    // Unselect delted entity.
               //    m_SelectedEntity = scene->GetAllEntities().at(0);
               //}

				
            }
            ImGui::EndChild();
            ImGui::Separator();
            // Draw a tree of entities.
            ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(26.f / 255.0f, 26.f / 255.0f, 26.f / 255.0f, 1));
            if (ImGui::BeginChild("Scene tree", ImGui::GetContentRegionAvail(), false))
            {
                if (ImGui::BeginTable("entity_table", 2, ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_NoBordersInBody))
                {
                    std::string icon = ICON_FA_EYE;
                    ImGui::TableSetupColumn(("    " + icon).c_str(), ImGuiTableColumnFlags_NoResize | ImGuiTableColumnFlags_IndentDisable | ImGuiTableColumnFlags_WidthFixed, 16);
                    ImGui::TableSetupColumn("Label", ImGuiTableColumnFlags_NoResize | ImGuiTableColumnFlags_IndentEnable);
                    ImGui::TableHeadersRow();
                    ImGui::TableNextRow();

                    std::vector<Entity> entities = scene->GetAllEntities();
                    for (Entity e : entities)
                    {
                        ImGuiTreeNodeFlags base_flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_SpanAvailWidth;
                        std::string name = e.GetComponent<NameComponent>().Name;
                        // If selected add selected flag.
                        if (Selection.Type == EditorSelectionType::Entity && Selection.Entity == e)
                            base_flags |= ImGuiTreeNodeFlags_Selected;

                        // Write in normal font.
                        ImGui::PushFont(normalFont);

                        // Small icons + name.
                        std::string label = ICON_FA_CIRCLE + std::string(" ") + name;

                        // Draw all entity without parents.
                        if (!e.GetComponent<ParentComponent>().HasParent)
                        {
                            // Recursively draw childrens.
                            DrawEntityTree(e);
                        }

                        // Pop font.
                        ImGui::PopFont();

                        // Right click menu
                        //if (ImGui::BeginPopupContextItem())
                        //    ImGui::EndPopup();
                    }
                }

				ImGui::EndTable();
				
            }
            ImGui::EndChild();
            ImGui::PopStyleColor();
            
            if (ImGui::BeginDragDropTarget()) // Drag n drop new prefab file into scene tree
            {
                if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("_Prefab"))
                {
                    char* file = (char*)payload->Data;
                    std::string fullPath = std::string(file, 256);
                    std::string relPath = FileSystem::AbsoluteToRelative(fullPath);

                    Entity newEntity = Engine::GetCurrentScene()->CreateEntity("New prefab Entity");
                    PrefabComponent& prefabComponent = newEntity.AddComponent<PrefabComponent>();
                    prefabComponent.PrefabInstance = Prefab::New(relPath);
                }
                ImGui::EndDragDropTarget();
            }

            // Deleted entity queue
            if (QueueDeletion.GetHandle() != -1)
            {
                Engine::GetCurrentScene()->DestroyEntity(QueueDeletion);

                // Clear Selection
                Selection = EditorSelection();

                QueueDeletion = Entity{ (entt::entity)-1, scene.get() };
            }
        }

        ImGui::End();
        ImGui::PopStyleVar();
    }

    bool EditorInterface::EntityContainsItself(Entity source, Entity target)
    {
        ParentComponent& targeParentComponent = target.GetComponent<ParentComponent>();
        if (!targeParentComponent.HasParent)
            return false;

        Entity currentParent = target.GetComponent<ParentComponent>().Parent;
        while (currentParent != source)
        {
            if (currentParent.GetComponent<ParentComponent>().HasParent)
                currentParent = currentParent.GetComponent<ParentComponent>().Parent;
            else
                return false;

            if (currentParent == source)
                return true;
        }
        return true;
    }

    bool LogErrors = true;
    bool LogWarnings = true;
    bool LogDebug = true;
    void EditorInterface::DrawLogger()
    {
        if (ImGui::Begin("Logger"))
        {
            ImGui::Checkbox("Errors", &LogErrors);
            ImGui::SameLine();
            ImGui::Checkbox("Warning", &LogWarnings);
            ImGui::SameLine();
            ImGui::Checkbox("Debug", &LogDebug);

            //ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
            //if (ImGui::BeginChild("Log window", ImGui::GetContentRegionAvail(), false))
            //{
                //ImGui::PopStyleVar();
            ImGuiTableFlags flags = ImGuiTableFlags_ScrollY | ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersV | ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable | ImGuiTableFlags_Hideable;

            if (ImGui::BeginTable("LogTable", 3, flags))
            {
                ImGui::TableSetupScrollFreeze(0, 1);
                ImGui::TableSetupColumn("Severity");
                ImGui::TableSetupColumn("Time");
                ImGui::TableSetupColumn("Message");
                ImGui::TableHeadersRow();
                ImGui::TableNextColumn();
                for (auto& l : Logger::GetLogs())
                {
                    if (l.type == LOG_TYPE::VERBOSE && !LogDebug)
                        continue;
                    if (l.type == LOG_TYPE::WARNING && !LogWarnings)
                        continue;
                    if (l.type == LOG_TYPE::CRITICAL && !LogErrors)
                        continue;
                    ImGui::Text("-");
                    ImGui::TableNextColumn();
                    ImGui::Text(l.time.c_str());
                    ImGui::TableNextColumn();
                    ImGui::TextWrapped(l.message.c_str());

                    ImGui::TableNextColumn();
                }
				if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
					ImGui::SetScrollHereY(1.0f);

                ImGui::EndTable();
            }


            //ImGui::EndChild();
        //}


        }
        ImGui::End();
    }

    void EditorInterface::Overlay()
    {
        // FIXME-VIEWPORT: Select a default viewport
        const float DISTANCE = 10.0f;
        int corner = 0;
        ImGuiIO& io = ImGui::GetIO();
        ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;
        if (corner != -1)
        {
            window_flags |= ImGuiWindowFlags_NoMove;
            ImGuiViewport* viewport = ImGui::GetWindowViewport();
            ImVec2 work_area_pos = ImGui::GetCurrentWindow()->Pos;   // Instead of using viewport->Pos we use GetWorkPos() to avoid menu bars, if any!
            ImVec2 work_area_size = ImGui::GetCurrentWindow()->Size;
            ImVec2 window_pos = ImVec2((corner & 1) ? (work_area_pos.x + work_area_size.x - DISTANCE) : (work_area_pos.x + DISTANCE), (corner & 2) ? (work_area_pos.y + work_area_size.y - DISTANCE) : (work_area_pos.y + DISTANCE));
            ImVec2 window_pos_pivot = ImVec2((corner & 1) ? 1.0f : 0.0f, (corner & 2) ? 1.0f : 0.0f);
            ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, window_pos_pivot);
            ImGui::SetNextWindowViewport(viewport->ID);
        }
        ImGui::SetNextWindowBgAlpha(0.35f); // Transparent background
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 32.0f);
        if (ImGui::Begin("Example: Simple overlay", &m_ShowOverlay, window_flags))
        {
            ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 100);
            if (ImGui::Button(ICON_FA_ARROWS_ALT) || Input::IsKeyPressed(GLFW_KEY_W))
            {
                CurrentOperation = ImGuizmo::OPERATION::TRANSLATE;
            }
            ImGui::SameLine();
            if (ImGui::Button(ICON_FA_SYNC_ALT) || Input::IsKeyPressed(GLFW_KEY_E))
            {
                CurrentOperation = ImGuizmo::OPERATION::ROTATE;
            }
            ImGui::SameLine();
            if (ImGui::Button(ICON_FA_EXPAND_ALT) || Input::IsKeyPressed(GLFW_KEY_R))
            {
                CurrentOperation = ImGuizmo::OPERATION::SCALE;
            }
            ImGui::PopStyleVar();
        }
        ImGui::PopStyleVar();
        ImGui::End();

        
        int corner2 = 1;

        window_flags |= ImGuiWindowFlags_NoMove;
        ImGuiViewport* viewport = ImGui::GetWindowViewport();
        ImVec2 work_area_pos = ImGui::GetCurrentWindow()->Pos;   // Instead of using viewport->Pos we use GetWorkPos() to avoid menu bars, if any!
        ImVec2 work_area_size = ImGui::GetCurrentWindow()->Size;
        ImVec2 window_pos = ImVec2((corner2 & 1) ? (work_area_pos.x + work_area_size.x - DISTANCE) : (work_area_pos.x + DISTANCE), (corner2 & 2) ? (work_area_pos.y + work_area_size.y - DISTANCE) : (work_area_pos.y + DISTANCE));
        ImVec2 window_pos_pivot = ImVec2((corner2 & 1) ? 1.0f : 0.0f, (corner2 & 2) ? 1.0f : 0.0f);
        ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, window_pos_pivot);
        ImGui::SetNextWindowViewport(viewport->ID);
        ImGui::SetNextWindowBgAlpha(0.35f); // Transparent background
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 32.0f);
        ImGui::SetNextWindowSize(ImVec2(25, ImGui::GetContentRegionAvail().y - DISTANCE * 2.0));
        if (ImGui::Begin("Controls", &m_ShowOverlay, window_flags))
        {
            const auto& editorCam = Engine::GetCurrentScene()->m_EditorCamera;
            const float camSpeed = editorCam->Speed;

            const float maxSpeed = 50.0f;
            const float minSpeed = 0.1f;
            const float normalizedSpeed = camSpeed / maxSpeed;

            ImVec2 start = ImGui::GetWindowPos();
            ImVec2 end = start + ImGui::GetWindowSize();

            ImVec2 startOffset = ImVec2(start.x, end.y - (normalizedSpeed * ImGui::GetWindowHeight()));

            ImGui::GetWindowDrawList()->AddRectFilled(startOffset, end, IM_COL32(255, 255, 255, 180), 200.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 100);
            ImGui::PopStyleVar();
        }

        ImGui::PopStyleVar();
        ImGui::End();
    }

    void NewProject()
    {
        if (Engine::GetProject())
            Engine::GetProject()->Save();

        std::string selectedProject = FileDialog::SaveFile("Project file\0*.project");
        if (selectedProject == "") // Hit cancel
            return;

        Ref<Project> project = Project::New("Unnamed project", "no description", selectedProject + ".project");
        Engine::LoadProject(project);
        Engine::LoadScene(Scene::New());
    }


    ProjectInterface pInterface;
    void OpenProject()
    {
        // Parse the project and load it.
        std::string projectPath = FileDialog::OpenFile("Project file\0*.project");

        if (projectPath == "") // Hit cancel.
            return;

        FileSystem::SetRootDirectory(projectPath + "/../");
        Ref<Project> project = Project::New();
        if (!project->Deserialize(FileSystem::ReadFile(projectPath, true)))
        {
            Logger::Log("Error loading project: " + projectPath, CRITICAL);
            return;
        }

        project->FullPath = projectPath;
        Engine::LoadProject(project);

        pInterface.m_CurrentProject = project;

        Engine::GetCurrentWindow()->SetTitle("Nuake Engine - Editing " + project->Name); 
    }

    void OpenScene()
    {
        // Parse the project and load it.
        std::string projectPath = FileDialog::OpenFile(".scene");

        Ref<Scene> scene = Scene::New();
        if (!scene->Deserialize(FileSystem::ReadFile(projectPath, true))) 
        {
            Logger::Log("Error failed loading scene: " + projectPath, CRITICAL);
            return;
        }

        scene->Path = FileSystem::AbsoluteToRelative(projectPath);
        Engine::LoadScene(scene);
    }

    void EditorInterface::DrawMenuBar()
    {
        if (ImGui::BeginMainMenuBar())
        {
            if (ImGui::BeginMenu("File"))
            {
                if (ImGui::MenuItem("New project", "CTRL+N"))
                {
                    NewProject();

                    Selection = EditorSelection();
                }
                if (ImGui::MenuItem("Open...", "CTRL+O"))
                {
                    OpenProject();

                    Selection = EditorSelection();
                }
                if (ImGui::MenuItem("Save", "CTRL+S"))
                {
                    Engine::GetProject()->Save();
                    Engine::GetCurrentScene()->Save();

                    Selection = EditorSelection();
                }
                if (ImGui::MenuItem("Save as...", "CTRL+SHIFT+S"))
                {
                    std::string savePath = FileDialog::SaveFile("*.project");
                    Engine::GetProject()->SaveAs(savePath);

                    Selection = EditorSelection();
                }
                ImGui::Separator();
                if (ImGui::MenuItem("Set current scene as default"))
                {
                    Engine::GetProject()->DefaultScene = Engine::GetCurrentScene();
                }
                ImGui::Separator();
                if (ImGui::MenuItem("New scene"))
                {
                    Engine::LoadScene(Scene::New());
                    Selection = EditorSelection();
                }
                if (ImGui::MenuItem("Open scene...", "CTRL+SHIFT+O"))
                {
                    OpenScene();
                    Selection = EditorSelection();
                }
                if (ImGui::MenuItem("Save scene", "CTR+SHIFT+L+S"))
                {
                    Engine::GetCurrentScene()->Save();
                    Selection = EditorSelection();
                }
                if (ImGui::MenuItem("Save scene as...", "CTRL+SHIFT+S"))
                {
                    std::string savePath = FileDialog::SaveFile("*.scene");
                    Engine::GetCurrentScene()->SaveAs(savePath);

                    Selection = EditorSelection();
                }

                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Edit"))
            {
                if (ImGui::MenuItem("Undo", "CTRL+Z")) {}
                if (ImGui::MenuItem("Redo", "CTRL+Y", false, false)) {}  // Disabled item
                ImGui::Separator();
                if (ImGui::MenuItem("Cut", "CTRL+X")) {}
                if (ImGui::MenuItem("Copy", "CTRL+C")) {}
                if (ImGui::MenuItem("Paste", "CTRL+V")) {}
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("View"))
            {
                if (ImGui::MenuItem("Lighting", NULL, true)) {}
                if (ImGui::MenuItem("Draw grid", NULL, m_DrawGrid))
                    m_DrawGrid = !m_DrawGrid;
                if (ImGui::MenuItem("Draw Axis", NULL, m_DrawAxis))
                    m_DrawAxis = !m_DrawAxis;
                if (ImGui::MenuItem("Draw collisions", NULL, m_DebugCollisions))
                {
                    m_DebugCollisions = !m_DebugCollisions;
                    PhysicsManager::Get()->SetDrawDebug(m_DebugCollisions);
                }

                if (ImGui::MenuItem("Settings", NULL)) {}
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Entity"))
            {
                if (ImGui::BeginMenu("Create new"))
                {
                    if (ImGui::MenuItem("Empty"))
                    {
                        auto ent = Engine::GetCurrentScene()->CreateEntity("Empty entity");
                    }
                    if (ImGui::MenuItem("Light"))
                    {
                        auto ent = Engine::GetCurrentScene()->CreateEntity("Light");
                        ent.AddComponent<LightComponent>();
                    }
                    if (ImGui::MenuItem("Camera"))
                    {
                        auto ent = Engine::GetCurrentScene()->CreateEntity("Camera");
                        ent.AddComponent<CameraComponent>();
                    }
                    if (ImGui::MenuItem("Rigidbody"))
                    {
                        auto ent = Engine::GetCurrentScene()->CreateEntity("Rigidbody");
                        ent.AddComponent<RigidBodyComponent>();
                    }
                    if (ImGui::MenuItem("Trenchbroom map"))
                    {
                        auto ent = Engine::GetCurrentScene()->CreateEntity("Trenchbroom map");
                        ent.AddComponent<QuakeMapComponent>();
                    }
                    if (ImGui::MenuItem("Model"))
                    {
                        auto ent = Engine::GetCurrentScene()->CreateEntity("Model");
                        ent.AddComponent<ModelComponent>();
                    }
                    ImGui::EndMenu();
                }
                if (ImGui::MenuItem("Delete selected", NULL));

                if (ImGui::MenuItem("Duplicate selected", NULL)) {}
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Debug"))
            {
                if (ImGui::MenuItem("Show ImGui demo", NULL, m_ShowImGuiDemo)) m_ShowImGuiDemo = !m_ShowImGuiDemo;
                if (ImGui::MenuItem("Rebuild Shaders", NULL))
                {
                    Nuake::Logger::Log("Rebuilding Shaders...");
                    Nuake::ShaderManager::RebuildShaders();
                    Nuake::Logger::Log("Shaders Rebuilt.");
                }
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Quit")) ImGui::EndMenu();
            ImGui::EndMainMenuBar();
        }
    }

    Ref<Scene> SceneSnapshot;
    
    void EditorInterface::Draw()
    {
        Init();

        if (!Engine::GetProject())
        {
            _WelcomeWindow->Draw();
            return;
        }

        pInterface.m_CurrentProject = Engine::GetProject();

        DrawMenuBar();

		pInterface.DrawEntitySettings();
        DrawViewport();
        DrawSceneTree();
        SelectionPanel.Draw(Selection);
        DrawLogger();

        filesystem->Draw();
        filesystem->DrawDirectoryExplorer();

        if (m_ShowImGuiDemo)
            ImGui::ShowDemoWindow();

        if (ImGui::Begin("Toolbar", 0, ImGuiWindowFlags_NoScrollbar | ImGuiDockNodeFlags_AutoHideTabBar | ImGuiWindowFlags_NoDecoration))
        {
            float availWidth = ImGui::GetContentRegionAvailWidth();
            float windowWidth = ImGui::GetWindowWidth();

            float used = windowWidth - availWidth;
            float half = windowWidth / 2.0;
            float needed = half - used;
            ImGui::Dummy(ImVec2(needed, 10));
            ImGui::SameLine();
            if (ImGui::Button(ICON_FA_PLAY))
            {
                SceneSnapshot = Engine::GetCurrentScene()->Copy();
                Engine::EnterPlayMode();
            }

            ImGui::SameLine();

            if (ImGui::Button(ICON_FA_STOP) || Input::IsKeyPressed(297))
            {
                Engine::ExitPlayMode();

                Engine::LoadScene(SceneSnapshot);
                Selection = EditorSelection();
            }
        }
        ImGui::End();
    }

    void EditorInterface::Update(float ts)
    {
        if (!Engine::GetCurrentScene())
        {
            return;
        }

        auto& editorCam = Engine::GetCurrentScene()->m_EditorCamera;
        editorCam->Update(ts, m_IsHoveringViewport);

        const bool entityIsSelected = Selection.Type == EditorSelectionType::Entity && Selection.Entity.IsValid();
        if (entityIsSelected && Input::IsKeyPressed(GLFW_KEY_F))
        {
            editorCam->IsMoving = true;
            editorCam->TargetPos = Selection.Entity.GetComponent<TransformComponent>().GetGlobalPosition();
        }

        if (entityIsSelected && Input::IsKeyPressed(GLFW_KEY_ESCAPE))
        {
            Selection = EditorSelection();
        }
    }

    void EditorInterface::BuildFonts()
    {
        ImGui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign, ImVec2(0.5f, 0.5f));
        FontManager::LoadFonts();
    }
}

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

#include <glad/glad.h>

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
#include "LoadingSplash.h"

#include "src/Rendering/SceneRenderer.h"
#include <dependencies/glfw/include/GLFW/glfw3.h>
#include <src/Rendering/Buffers/Framebuffer.h>
#include "UIDemoWindow.h"
#include <src/Audio/AudioManager.h>

#include <src/UI/ImUI.h>

#include <src/Resource/StaticResources.h>

namespace Nuake {
    Ref<UI::UserInterface> userInterface;
    ImFont* normalFont;
    ImFont* boldFont;
    ImFont* EditorInterface::bigIconFont;
    
    EditorInterface::EditorInterface()
    {
        Logger::Log("Creating editor windows", "window", CRITICAL);
        filesystem = new FileSystemUI(this);
        _WelcomeWindow = new WelcomeWindow(this);
        _audioWindow = new AudioWindow();

        Logger::Log("Building fonts", "window", CRITICAL);
        BuildFonts();

        Logger::Log("Loading imgui from mem", "window", CRITICAL);
        using namespace Nuake::StaticResources;
        ImGui::LoadIniSettingsFromMemory((const char*)StaticResources::Resources_default_layout_ini);
    }

    void EditorInterface::Init()
    {
        ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
        static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_AutoHideTabBar;
        ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->Pos);
        ImGui::SetNextWindowSize(viewport->Size);
        ImGui::SetNextWindowViewport(viewport->ID);

        ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
        ImGui::DockSpaceOverViewport(viewport, dockspace_flags);
        //this->filesystem = FileSystemUI();
    }


    ImVec2 LastSize = ImVec2();
    void EditorInterface::DrawViewport()
    {
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(4, 4));
        std::string name = ICON_FA_GAMEPAD + std::string("  Scene");
        if (ImGui::Begin(name.c_str()))
        {
            ImGui::PopStyleVar();

            Overlay();
            ImGuizmo::BeginFrame();

            float availWidth = ImGui::GetContentRegionAvail().x;
            float windowWidth = ImGui::GetWindowWidth();

            float used = windowWidth - availWidth;
            float half = windowWidth / 2.0;
            float needed = half - used;

            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(2, 2));
            if (ImGui::Button(ICON_FA_PLAY, ImVec2(30, 30)) || (Input::IsKeyPressed(GLFW_KEY_F5) && !Engine::IsPlayMode()))
            {
                SceneSnapshot = Engine::GetCurrentScene()->Copy();
                Engine::EnterPlayMode();
            }

            ImGui::SameLine();

            if ((ImGui::Button(ICON_FA_STOP, ImVec2(30, 30)) || Input::IsKeyPressed(GLFW_KEY_F8)) && Engine::IsPlayMode())
            {
                Engine::ExitPlayMode();

                Engine::LoadScene(SceneSnapshot);
                Selection = EditorSelection();
            }

            ImGui::SameLine();

            ImGui::Dummy(ImVec2(ImGui::GetContentRegionAvail().x - 120, 30));

            ImGui::SameLine();

            const auto& io = ImGui::GetIO();
            float frameTime = 1000.0f / io.Framerate;
            int fps = (int) io.Framerate;
            ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 0.0f);
            ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.1, 0.1, 0.1, 1));
            ImGui::BeginChild("FPS", ImVec2(60, 30), false);

            std::string text = std::to_string(fps) + " fps";
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x / 1.25 - ImGui::CalcTextSize(text.c_str()).x
                - ImGui::GetScrollX() - 2 * ImGui::GetStyle().ItemSpacing.x);
            ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 15 - (ImGui::CalcTextSize(text.c_str()).y) / 2.0);
            ImGui::Text(text.c_str());

            ImGui::EndChild();
            ImGui::SameLine();
            ImGui::BeginChild("frametime", ImVec2(60, 30), false);
            std::ostringstream out;
            out.precision(2);
            out << std::fixed << frameTime;
            text = out.str() + " ms";
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x / 1.25 - ImGui::CalcTextSize(text.c_str()).x
                - ImGui::GetScrollX() - 2 * ImGui::GetStyle().ItemSpacing.x);
            ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 15 - (ImGui::CalcTextSize(text.c_str()).y) / 2.0);
            ImGui::Text(text.c_str());

            ImGui::EndChild();

            ImGui::PopStyleColor();
            ImGui::PopStyleVar();

            ImGui::PopStyleVar(1);
            ImGuizmo::SetOrthographic(false);
            ImVec2 regionAvail = ImGui::GetContentRegionAvail();
            Vector2 viewportPanelSize = glm::vec2(regionAvail.x, regionAvail.y);

            Ref<FrameBuffer> framebuffer = Engine::GetCurrentWindow()->GetFrameBuffer();
            if (framebuffer->GetSize() != viewportPanelSize)
                framebuffer->QueueResize(viewportPanelSize);

            Ref<Texture> texture = framebuffer->GetTexture();
            ImVec2 imagePos = ImGui::GetWindowPos() + ImGui::GetCursorPos();

            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
            m_ViewportPos = { imagePos.x, imagePos.y };
            ImGui::Image((void*)texture->GetID(), regionAvail, ImVec2(0, 1), ImVec2(1, 0));
            ImGui::PopStyleVar();

            const Vector2& mousePos = Input::GetMousePosition();
            const ImVec2& windowPos = ImGui::GetWindowPos() + ImVec2(0, 30.0);
            const ImVec2& windowSize = ImGui::GetWindowSize() - ImVec2(0, 30.0);
            const bool isInsideWidth = mousePos.x > windowPos.x && mousePos.x < windowPos.x + windowSize.x;
            const bool isInsideHeight = mousePos.y > windowPos.y && mousePos.y < windowPos.y + windowSize.y;
            m_IsHoveringViewport = isInsideWidth && isInsideHeight;

            ImGuizmo::SetDrawlist();
            ImGuizmo::AllowAxisFlip(true);
            ImGuizmo::SetRect(imagePos.x, imagePos.y + 0.0f, viewportPanelSize.x, viewportPanelSize.y);

            if (m_DrawGrid && !Engine::IsPlayMode())
            {
                ImGuizmo::DrawGrid(glm::value_ptr(Engine::GetCurrentScene()->GetCurrentCamera()->GetTransform()),
                    glm::value_ptr(Engine::GetCurrentScene()->GetCurrentCamera()->GetPerspective()),
                    glm::value_ptr(glm::identity<glm::mat4>()), 100.f);
            }

            if (Selection.Type == EditorSelectionType::Entity && !Engine::IsPlayMode())
            {
                TransformComponent& tc = Selection.Entity.GetComponent<TransformComponent>();
                Matrix4 transform = tc.GetGlobalTransform();
                const auto& editorCam = Engine::GetCurrentScene()->GetCurrentCamera();
                Matrix4 cameraView = editorCam->GetTransform();
                Matrix4 cameraProjection = editorCam->GetPerspective();

                // Imguizmo calculates the delta from the gizmo,
                ImGuizmo::Manipulate(
                    glm::value_ptr(cameraView),
                    glm::value_ptr(cameraProjection),
                    CurrentOperation, CurrentMode, 
                    glm::value_ptr(transform)
                );

                if (ImGuizmo::IsUsing())
                {
                    // Since imguizmo returns a transform in global space and we want the local transform,
                    // we need to multiply by the inverse of the parent's global transform in order to revert
                    // the changes from the parent transform.
                    Matrix4 localTransform = Matrix4(transform);
                    ParentComponent& parent = Selection.Entity.GetComponent<ParentComponent>();
                    if (parent.HasParent)
                    {
                        const auto& parentTransformComponent = parent.Parent.GetComponent<TransformComponent>();
                        const Matrix4& parentTransform = parentTransformComponent.GetGlobalTransform();
                        localTransform = glm::inverse(parentTransform) * localTransform;
                    }

                    // Decompose local transform
                    float decomposedPosition[3];
                    float decomposedEuler[3];
                    float decomposedScale[3];
                    ImGuizmo::DecomposeMatrixToComponents(glm::value_ptr(localTransform), decomposedPosition, decomposedEuler, decomposedScale);
                    
                    const auto& localPosition = Vector3(decomposedPosition[0], decomposedPosition[1], decomposedPosition[2]);
                    const auto& localScale = Vector3(decomposedScale[0], decomposedScale[1], decomposedScale[2]);

                    localTransform[0] /= localScale.x;
                    localTransform[1] /= localScale.y;
                    localTransform[2] /= localScale.z;
                    const auto& rotationMatrix = Matrix3(localTransform);
                    const Quat& localRotation = glm::normalize(Quat(rotationMatrix));

                    const Matrix4& rotationMatrix4 = glm::mat4_cast(localRotation);
                    const Matrix4& scaleMatrix = glm::scale(Matrix4(1.0f), localScale);
                    const Matrix4& translationMatrix = glm::translate(Matrix4(1.0f), localPosition);
                    const Matrix4& newLocalTransform = translationMatrix * rotationMatrix4 * scaleMatrix;

                    tc.Translation = localPosition;

                    if (CurrentOperation != ImGuizmo::SCALE)
                    {
                        tc.Rotation = localRotation;
                    }

                    tc.Scale = localScale;
                    tc.LocalTransform = newLocalTransform;
                    tc.Dirty = true;
                }
            }

            if (m_IsHoveringViewport && !m_IsViewportFocused && Input::IsMouseButtonPressed(GLFW_MOUSE_BUTTON_2))
            {
                ImGui::FocusWindow(ImGui::GetCurrentWindow());
            }

            m_IsViewportFocused = ImGui::IsWindowFocused();

            if (m_IsHoveringViewport && Input::IsMouseButtonPressed(GLFW_MOUSE_BUTTON_1) && !ImGuizmo::IsUsing() && m_IsViewportFocused)
            {
                const auto windowPosNuake = Vector2(windowPos.x, windowPos.y);
                auto& gbuffer = Engine::GetCurrentScene()->m_SceneRenderer->GetGBuffer();
                auto pixelPos = Input::GetMousePosition() - windowPosNuake;
                pixelPos.y = gbuffer.GetSize().y - pixelPos.y; // imgui coords are inverted on the Y axis

                gbuffer.Bind();

                if (const int result = gbuffer.ReadPixel(3, pixelPos); result > 0)
                {
                    auto ent = Entity{ (entt::entity)(result - 1), Engine::GetCurrentScene().get() };
                    if (ent.IsValid())
                    {
                        Selection = EditorSelection(ent);
                    }
                }
                else
                {
                    Selection = EditorSelection(); // None
                }

                gbuffer.Unbind();
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
        ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2{ 0.0f, 0.0f });

        ImGuiTreeNodeFlags base_flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_FramePadding | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_SpanFullWidth;

		NameComponent& nameComponent = e.GetComponent<NameComponent>();
		std::string name = nameComponent.Name;
        ParentComponent& parent = e.GetComponent<ParentComponent>();

        if (Selection.Type == EditorSelectionType::Entity && Selection.Entity == e)
            base_flags |= ImGuiTreeNodeFlags_Selected;

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
        
        bool open = ImGui::TreeNodeEx(name.c_str(), base_flags);

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
                    Vector3 camDirection = entity.GetComponent<CameraComponent>().CameraInstance->GetDirection();
                    camDirection.z *= -1.0f;
                    editorCam->SetTransform(glm::inverse(entity.GetComponent<TransformComponent>().GetGlobalTransform()));
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
                if (!String::EndsWith(savePath, ".prefab"))
                {
                    savePath += ".prefab";
                }

                if (!savePath.empty()) 
                {
                    newPrefab->SaveAs(savePath);
                    Selection.Entity.AddComponent<PrefabComponent>().PrefabInstance = newPrefab;
                    FileSystem::Scan();
                    FileSystemUI::m_CurrentDirectory = FileSystem::RootDirectory;
                }
            }
            ImGui::EndPopup();
        }
        
        ImGui::TableNextColumn();

        ImGui::TextColored(ImVec4(0.5, 0.5, 0.5, 1.0), GetEntityTypeName(e).c_str());

        ImGui::TableNextColumn();
        {
            bool& isVisible = e.GetComponent<VisibilityComponent>().Visible;
            std::string visibilityIcon = isVisible ? ICON_FA_EYE : ICON_FA_EYE_SLASH;
            ImGui::PushStyleColor(ImGuiCol_Button, { 0, 0, 0, 0 });
            if (ImGui::Button(visibilityIcon.c_str(), {40, 0}))
            {
                isVisible = !isVisible;
            }
            ImGui::PopStyleColor();
        }
       
        if (open)
        {
            // Caching list to prevent deletion while iterating.
            std::vector<Entity> childrens = parent.Children;
            for (auto& c : childrens)
                DrawEntityTree(c);

            ImGui::TreePop();
        }
        
        ImGui::PopStyleVar();
        ImGui::PopFont();
    }

#define BEGIN_COLLAPSE_HEADER(names) \
    UIFont* names##_boldFont = new UIFont(Fonts::Bold); \
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.f, 0.f)); \
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.f, 8.f)); \
    bool names##_Opened = ImGui::CollapsingHeader((std::string("  ") + #names).c_str()); \
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

        if (ImGui::Begin((ICON_FA_MOUNTAIN + std::string("  Environment")).c_str()))
        {
            BEGIN_COLLAPSE_HEADER(SKY);
                if (ImGui::BeginTable("EnvTable", 3, ImGuiTableFlags_BordersInner | ImGuiTableFlags_SizingStretchProp))
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
                            env->ProceduralSkybox->SunDirection = glm::normalize(sunDirection);
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

                    ImGui::EndTable();
                }
            END_COLLAPSE_HEADER()

            BEGIN_COLLAPSE_HEADER(BLOOM)
                if (ImGui::BeginTable("BloomTable", 3, ImGuiTableFlags_BordersInner | ImGuiTableFlags_SizingStretchProp))
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
                    ImGui::EndTable();
                }
            END_COLLAPSE_HEADER()

            BEGIN_COLLAPSE_HEADER(VOLUMETRIC)
                if (ImGui::BeginTable("EnvTable", 3, ImGuiTableFlags_BordersInner | ImGuiTableFlags_SizingStretchProp))
                {
                    ImGui::TableSetupColumn("name", 0, 0.3);
                    ImGui::TableSetupColumn("set", 0, 0.6);
                    ImGui::TableSetupColumn("reset", 0, 0.1);

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

                    ImGui::EndTable();
                }
            END_COLLAPSE_HEADER()

            BEGIN_COLLAPSE_HEADER(SSAO)
                if (ImGui::BeginTable("EnvTable", 3, ImGuiTableFlags_BordersInner | ImGuiTableFlags_SizingStretchProp))
                {
                    ImGui::TableSetupColumn("name", 0, 0.3);
                    ImGui::TableSetupColumn("set", 0, 0.6);
                    ImGui::TableSetupColumn("reset", 0, 0.1);

                    ImGui::TableNextColumn();
                    {
                        // Title
                        ImGui::Text("SSAO");
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

                        ImGui::DragFloat("##SSAORadius", &env->mSSAO->Radius, 0.001f, 0.0f, 1.0f);
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
                        if (ImGui::Button(resetBloomThreshold.c_str())) env->mSSAO->Bias = 0.025f;
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

                    ImGui::EndTable();
                }
            END_COLLAPSE_HEADER()

            BEGIN_COLLAPSE_HEADER(SSR)
                if (ImGui::BeginTable("EnvTable", 3, ImGuiTableFlags_BordersInner | ImGuiTableFlags_SizingStretchProp))
                {
                    ImGui::TableSetupColumn("name", 0, 0.3);
                    ImGui::TableSetupColumn("set", 0, 0.6);
                    ImGui::TableSetupColumn("reset", 0, 0.1);

                    SSR* ssr = scene->m_SceneRenderer->mSSR.get();
                    {
                        ImGui::TableNextColumn();
                        // Title
                        ImGui::Text("SSR");
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
                        ImGui::Text("DOF Enabled");
                        ImGui::TableNextColumn();

                        ImGui::Checkbox("##dofEnabled", &env->DOFEnabled);
                        ImGui::TableNextColumn();

                        // Reset button
                        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1, 1, 1, 0));
                        std::string resetSSR = ICON_FA_UNDO + std::string("##resetrBarrelDistortionEnabled");
                        if (ImGui::Button(resetSSR.c_str())) env->DOFEnabled = false;
                        ImGui::PopStyleColor();
                    }

                    ImGui::EndTable();
                }
            END_COLLAPSE_HEADER()

                BEGIN_COLLAPSE_HEADER(BARREL_DISTORTION)
                if (ImGui::BeginTable("EnvTable", 3, ImGuiTableFlags_BordersInner | ImGuiTableFlags_SizingStretchProp))
                {
                    ImGui::TableSetupColumn("name", 0, 0.3);
                    ImGui::TableSetupColumn("set", 0, 0.6);
                    ImGui::TableSetupColumn("reset", 0, 0.1);

                    
                    {
                        ImGui::TableNextColumn();
                        // Title
                        ImGui::Text("Barrel Distortion");
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
                        ImGui::Text("Vignette Enabled");
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
        }
        ImGui::End();

        std::string title = ICON_FA_TREE + std::string(" Hierarchy");
        if (ImGui::Begin(title.c_str()))
        {
            // Draw a tree of entities.
            ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(26.f / 255.0f, 26.f / 255.0f, 26.f / 255.0f, 1));
            if (ImGui::BeginChild("Scene tree", ImGui::GetContentRegionAvail(), false))
            {
                ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, { 8, 4 });
                if (ImGui::BeginTable("entity_table", 3, ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_NoPadInnerX | ImGuiTableFlags_NoPadOuterX))
                {
                    ImGui::TableSetupColumn("Label", ImGuiTableColumnFlags_IndentEnable);
                    ImGui::TableSetupColumn("Type", ImGuiTableColumnFlags_IndentEnable);
                    ImGui::TableSetupColumn("Visibility", ImGuiTableColumnFlags_NoResize | ImGuiTableColumnFlags_IndentDisable | ImGuiTableColumnFlags_WidthFixed);
                    ImGui::TableHeadersRow();

                    ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(0, 0));
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
                    ImGui::PopStyleVar();
                }
				ImGui::EndTable();
                ImGui::PopStyleVar();
				
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
                    Prefab::New(relPath);
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
    bool AutoScroll = true;
    void EditorInterface::DrawLogger()
    {
        if (ImGui::Begin("Logger"))
        {
            ImGui::Checkbox("Errors", &LogErrors);
            ImGui::SameLine();
            ImGui::Checkbox("Warning", &LogWarnings);
            ImGui::SameLine();
            ImGui::Checkbox("Debug", &LogDebug);
            ImGui::SameLine();
            ImGui::Checkbox("Autoscroll", &AutoScroll);
            //ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
            //if (ImGui::BeginChild("Log window", ImGui::GetContentRegionAvail(), false))
            //{
                //ImGui::PopStyleVar();
            ImGuiTableFlags flags = ImGuiTableFlags_ScrollY | ImGuiTableFlags_RowBg | ImGuiTableFlags_Hideable;
            if (ImGui::BeginTable("LogTable", 3, flags))
            {
                ImGui::TableSetupColumn("Severity", ImGuiTableColumnFlags_WidthFixed, 64.0f);
                ImGui::TableSetupColumn("Time", ImGuiTableColumnFlags_WidthFixed, 64.0f);
                ImGui::TableSetupColumn("Message", ImGuiTableColumnFlags_WidthStretch, 1.0f);
                ImGui::TableNextColumn();
                ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(4, 4));
                for (auto& l : Logger::GetLogs())
                {
                    if (l.type == LOG_TYPE::VERBOSE && !LogDebug)
                        continue;
                    if (l.type == LOG_TYPE::WARNING && !LogWarnings)
                        continue;
                    if (l.type == LOG_TYPE::CRITICAL && !LogErrors)
                        continue;

                    std::string severityText = "";
                    if (l.type == LOG_TYPE::VERBOSE)
                        severityText = "verbose";
                    else if (l.type == LOG_TYPE::WARNING)
                        severityText = "warning";
                    else
                        severityText = "critical";

                    ImVec4 colorGreen = ImVec4(0.59, 0.76, 0.47, 1.0);
                    ImGui::PushStyleColor(ImGuiCol_Text, colorGreen);
                    ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, ImGui::GetColorU32(ImVec4(0.59, 0.76, 0.47, 0.2)), -1);
                    const std::string timeString = " [" + l.time + "]";
                    ImGui::Text(timeString.c_str());
                    ImGui::PopStyleColor();

                    ImGui::TableNextColumn();

                    ImVec4 colorBlue = ImVec4(98 / 255.0, 174 / 255.0, 239 / 255.0, 1.);
                    ImGui::PushStyleColor(ImGuiCol_Text, colorBlue);
                    ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, ImGui::GetColorU32(ImVec4(98 / 255.0, 174 / 255.0, 239 / 255.0, 0.2)), -1);
                    ImGui::Text(l.logger.c_str());
                    ImGui::PopStyleColor();

                    ImGui::TableNextColumn();

                    ImVec4 color = ImVec4(1, 1, 1, 1.0);
                    ImGui::PushStyleColor(ImGuiCol_Text, color);
                    ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, ImGui::GetColorU32(ImVec4(1, 1, 1, 0.0)), -1);
                    ImGui::TextWrapped(l.message.c_str());
                    ImGui::PopStyleColor();

                    ImGui::TableNextColumn();
                }
                ImGui::PopStyleVar();

                if (AutoScroll)
                {
                    ImGui::SetScrollY(ImGui::GetScrollMaxY());
                }

                ImGui::EndTable();
            }
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
            ImGui::SetNextWindowPos(window_pos + ImVec2(0, 36), ImGuiCond_Always, window_pos_pivot);
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
        ImGui::SetNextWindowPos(window_pos + ImVec2(0, 30), ImGuiCond_Always, window_pos_pivot);
        ImGui::SetNextWindowViewport(viewport->ID);
        ImGui::SetNextWindowBgAlpha(0.35f); // Transparent background
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 32.0f);
        ImGui::SetNextWindowSize(ImVec2(25, ImGui::GetContentRegionAvail().y - DISTANCE * 2.0 - 30.0));
        if (ImGui::Begin("Controls", &m_ShowOverlay, window_flags))
        {
            const auto& editorCam = Engine::GetCurrentScene()->m_EditorCamera;
            const float camSpeed = editorCam->Speed;

            const float maxSpeed = 50.0f;
            const float minSpeed = 0.1f;
            const float normalizedSpeed = camSpeed / maxSpeed;

            ImVec2 start = ImGui::GetWindowPos() - ImVec2(0.0, 15.0) ;
            ImVec2 end = start + ImGui::GetWindowSize() - ImVec2(0, 10.0);
            ImVec2 startOffset = ImVec2(start.x , end.y - (normalizedSpeed * (ImGui::GetWindowHeight() -10.0)));

            ImGui::GetWindowDrawList()->AddRectFilled(startOffset + ImVec2(0, 10.0), end + ImVec2(0.0, 15.0), IM_COL32(255, 255, 255, 180), 32.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 100);
            ImGui::PopStyleVar();
        }

        ImGui::PopStyleVar();
        ImGui::End();
    }

    void NewProject()
    {
        if (Engine::GetProject() && Engine::GetProject()->FileExist())
            Engine::GetProject()->Save();
        
        std::string selectedProject = FileDialog::SaveFile("Project file\0*.project");
        
        if(!String::EndsWith(selectedProject, ".project"))
            selectedProject += ".project";
        
        if (selectedProject.empty()) // Hit cancel
            return;

        auto backslashSplits = String::Split(selectedProject, '\\');
        auto fileName = backslashSplits[backslashSplits.size() - 1];

        std::string finalPath = String::Split(selectedProject, '.')[0];
        
        // We need to create a folder
        if (const auto& dirPath = finalPath;
            !std::filesystem::create_directory(dirPath))
        {
            // Should we continue?
            Logger::Log("Failed creating project directory: " + dirPath);
        }

        finalPath += "\\" + fileName;
        
        Ref<Project> project = Project::New(String::Split(fileName, '.')[0], "no description", finalPath);
        Engine::LoadProject(project);
        Engine::LoadScene(Scene::New());

        Engine::GetCurrentWindow()->SetTitle("Nuake Engine - Editing " + project->Name); 
    }


    ProjectInterface pInterface;
    void OpenProject()
    {
        // Parse the project and load it.
        std::string projectPath = FileDialog::OpenFile("Project file\0*.project");

        if (projectPath == "") // Hit cancel.
            return;

        FileSystem::SetRootDirectory(FileSystem::GetParentPath(projectPath));
        Ref<Project> project = Project::New();
        if (!project->Deserialize(FileSystem::ReadFile(projectPath, true)))
        {
            Logger::Log("Error loading project: " + projectPath, "editor", CRITICAL);
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
            Logger::Log("Error failed loading scene: " + projectPath, "editor", CRITICAL);
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
                if (ImGui::MenuItem("Open..."))
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
                if (ImGui::MenuItem("Open scene...", "CTRL+O"))
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
                if (ImGui::MenuItem("Draw grid", NULL, m_DrawGrid))
                    m_DrawGrid = !m_DrawGrid;
                if (ImGui::MenuItem("Draw Axis", NULL, m_DrawAxis))
                    m_DrawAxis = !m_DrawAxis;
                if (ImGui::MenuItem("Draw collisions", NULL, m_DebugCollisions))
                {
                    m_DebugCollisions = !m_DebugCollisions;
                    PhysicsManager::Get().SetDrawDebug(m_DebugCollisions);
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

    bool isLoadingProject = false;
    bool isLoadingProjectQueue = false;
    UIDemoWindow m_DemoWindow;

    int frameCount = 2;
    void EditorInterface::Draw()
    {
        Init();

        if (isLoadingProjectQueue)
        {
            _WelcomeWindow->LoadQueuedProject();
            isLoadingProjectQueue = false;

            auto window = Window::Get();
            window->SetDecorated(true);
            window->SetSize({ 1900, 1000 });
            window->Center();
            frameCount = 0;
            return;
        }

        // Shortcuts
        if(ImGui::IsKeyDown(ImGuiKey_LeftCtrl))
        {
            if(ImGui::IsKeyPressed(ImGuiKey_S))
            {
                Engine::GetProject()->Save();
                Engine::GetCurrentScene()->Save();

                Selection = EditorSelection();
            }
            else if(ImGui::IsKeyPressed(ImGuiKey_O))
            {
                OpenScene();
                
                Selection = EditorSelection();
            }
            else if(ImGui::IsKeyDown(ImGuiKey_LeftShift) && ImGui::IsKeyPressed(ImGuiKey_S))
            {
                std::string savePath = FileDialog::SaveFile("*.project");
                Engine::GetProject()->SaveAs(savePath);

                Selection = EditorSelection();
            }
        }

        if (_WelcomeWindow->IsProjectQueued() && frameCount > 0)
        {
            // draw splash
            LoadingSplash::Get().Draw();

            frameCount--;
            if (frameCount == 0)
            {
                isLoadingProjectQueue = true;
            }

            return;
        }

        if (!Engine::GetProject())
        {
            _WelcomeWindow->Draw();
            return;
        }

        pInterface.m_CurrentProject = Engine::GetProject();

        m_DemoWindow.Draw();

        _audioWindow->Draw();

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
    }

    void EditorInterface::Update(float ts)
    {
        if (!Engine::GetCurrentScene() || Engine::IsPlayMode())
        {
            return;
        }

        auto& editorCam = Engine::GetCurrentScene()->m_EditorCamera;

        if (m_IsViewportFocused)
        {
            editorCam->Update(ts, m_IsHoveringViewport);
        }

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

    std::string EditorInterface::GetEntityTypeName(const Entity& entity) const
    {
        std::string entityTypeName = "";
        
        if (entity.HasComponent<LightComponent>())
        {
            entityTypeName = "Light";
        }

        if (entity.HasComponent<RigidBodyComponent>())
        {
            entityTypeName = "Rigidbody";
        }

        if (entity.HasComponent<CharacterControllerComponent>())
        {
            entityTypeName = "Character Controller";
        }

        if (entity.HasComponent<BoneComponent>())
        {
            entityTypeName = "Bone";
        }

        if (entity.HasComponent<PrefabComponent>())
        {
            entityTypeName = "Prefab";
        }

        return entityTypeName;
    }

	bool EditorInterface::LoadProject(const std::string& projectPath)
	{
        FileSystem::SetRootDirectory(FileSystem::GetParentPath(projectPath));

        auto project = Project::New();
        auto projectFileData = FileSystem::ReadFile(projectPath, true);
        Logger::Log("Reading file project: " + projectFileData, "window", CRITICAL);
        try
        {
            Logger::Log("Starting deserializing", "window", CRITICAL);
            project->Deserialize(json::parse(projectFileData));
            project->FullPath = projectPath;

            Engine::LoadProject(project);

            filesystem->m_CurrentDirectory = Nuake::FileSystem::RootDirectory;
        }
        catch (std::exception exception)
        {
            Logger::Log("Error loading project: " + projectPath, "editor", CRITICAL);
            Logger::Log(exception.what());
            return false;
        }

        return true;
	}

}

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

#include "src/Physics/PhysicsManager.h"

#include "src/Vendors/glm/gtc/type_ptr.hpp"
#include "src/Vendors/glm/gtx/matrix_decompose.hpp"
#include "src/Resource/FontAwesome5.h"

#include <glad/glad.h>

#include "src/Scene/Scene.h"
#include "src/Scene/Components.h"
#include "src/Scene/Systems/QuakeMapBuilder.h"
#include "../UIComponents/Viewport.h"
#include <src/Resource/Prefab.h>
#include <src/Rendering/Shaders/ShaderManager.h>
#include "src/Rendering/Renderer.h"
#include "src/Core/Input.h"

#include "../Actions/EditorSelection.h"
#include "FileSystemUI.h"

#include <src/FileSystem/Directory.h>

#include "../Misc/InterfaceFonts.h"

#include "WelcomeWindow.h"
#include "LoadingSplash.h"

#include "src/Rendering/SceneRenderer.h"
#include <dependencies/glfw/include/GLFW/glfw3.h>
#include <src/Rendering/Buffers/Framebuffer.h>
#include "UIDemoWindow.h"
#include <src/Audio/AudioManager.h>

#include <src/UI/ImUI.h>
#include "src/FileSystem/FileSystem.h"
#include <src/Resource/StaticResources.h>
#include <src/Threading/JobSystem.h>
#include "../Commands/Commands/Commands.h"
#include <src/Resource/ModelLoader.h>
#include "../ScriptingContext/ScriptingContext.h"
#include <src/Scene/Components/BSPBrushComponent.h>

#include <src/FileSystem/FileDialog.h>

#include <Tracy.hpp>

namespace Nuake {
    
    ImFont* normalFont;
    ImFont* boldFont;
    ImFont* EditorInterface::bigIconFont;
    
    NuakeEditor::CommandBuffer* EditorInterface::mCommandBuffer;
    EditorSelection EditorInterface::Selection;

    int SelectedViewport = 0;
    bool displayVirtualCameraOverlay = false;
    Ref<FrameBuffer> virtualCamera;

    glm::vec3 DepthToWorldPosition(const glm::vec2& pixelPos, float depth, const glm::mat4& projectionMatrix, const glm::mat4& viewMatrix, const glm::vec2& viewportSize)
    {
        // Convert pixel position to normalized device coordinates (NDC)
        glm::vec2 ndcPos;
        ndcPos.x = (2.0f * pixelPos.x) / viewportSize.x - 1.0f;
        ndcPos.y = 1.0f - (2.0f * pixelPos.y) / viewportSize.y;  // Y-axis inversion

        // Depth value ranges from 0 to 1 in NDC space
        float ndcDepth = depth * 2.0f - 1.0f;

        // Construct the NDC position
        glm::vec4 ndcPos4(ndcPos.x, ndcPos.y, ndcDepth, 1.0f);

        // Compute the inverse of the projection and view matrices
        glm::mat4 invProj = glm::inverse(projectionMatrix);
        glm::mat4 invView = glm::inverse(viewMatrix);

        // Transform the NDC position to eye space
        glm::vec4 eyePos4 = invProj * ndcPos4;
        eyePos4.z = -1.0f;  // Set Z to -1 as it's on the near plane
        eyePos4 /= eyePos4.w;
        
        //eyePos4.w = 0.0f;

        // Transform the eye space position to world space
        glm::vec4 worldPos4 = invView * eyePos4;

        // Return the 3D world position
        glm::vec3 worldPos(worldPos4.x, worldPos4.y, worldPos4.z);
        return worldPos;
    }

    EditorInterface::EditorInterface(CommandBuffer& commandBuffer)
    {
        mCommandBuffer = &commandBuffer;

        Logger::Log("Creating editor windows", "window", VERBOSE);
        filesystem = new FileSystemUI(this);

        _WelcomeWindow = new WelcomeWindow(this);
        _NewProjectWindow = new NewProjectWindow(this);
        _audioWindow = new AudioWindow();
        m_ProjectSettingsWindow = new ProjectSettingsWindow();

        Logger::Log("Building fonts", "window", VERBOSE);
        BuildFonts();

        Logger::Log("Loading imgui from mem", "window", VERBOSE);
        using namespace Nuake::StaticResources;
        ImGui::LoadIniSettingsFromMemory((const char*)StaticResources::Resources_default_layout_ini);

        virtualCamera = CreateRef<FrameBuffer>(true, Vector2{ 640, 360 });
        virtualCamera->SetTexture(CreateRef<Texture>(Vector2{ 640, 360 }, GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE), GL_COLOR_ATTACHMENT0);
        //ScriptingContext::Get().Initialize();
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
    }

    ImVec2 LastSize = ImVec2();
    void EditorInterface::DrawViewport()
    {
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
        std::string name = ICON_FA_GAMEPAD + std::string("  Scene");
        if (ImGui::Begin(name.c_str()))
        {
            ImGui::PopStyleVar();

            Overlay();
            ImGuizmo::BeginFrame();
 
            ImGuizmo::SetOrthographic(false);
            ImVec2 regionAvail = ImGui::GetContentRegionAvail();
            Vector2 viewportPanelSize = glm::vec2(regionAvail.x, regionAvail.y);

            // This is important for make UI mouse coord relative to viewport
            Input::SetViewportDimensions(m_ViewportPos, viewportPanelSize);

            Ref<FrameBuffer> framebuffer = Engine::GetCurrentWindow()->GetFrameBuffer();
            if (framebuffer->GetSize() != viewportPanelSize * Engine::GetProject()->Settings.ResolutionScale)
                framebuffer->QueueResize(viewportPanelSize * Engine::GetProject()->Settings.ResolutionScale);

            Ref<Texture> texture = framebuffer->GetTexture();
            if (SelectedViewport == 1)
            {
                texture = Engine::GetCurrentScene()->m_SceneRenderer->GetGBuffer().GetTexture(GL_COLOR_ATTACHMENT0);
            }
            else if (SelectedViewport == 2)
            {
                texture = Engine::GetCurrentScene()->m_SceneRenderer->GetGBuffer().GetTexture(GL_COLOR_ATTACHMENT1);
            }
            else if (SelectedViewport == 3)
            {
                texture = Engine::GetCurrentScene()->m_SceneRenderer->GetScaledDepthTexture();
            }

            ImVec2 imagePos = ImGui::GetWindowPos() + ImGui::GetCursorPos();

            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
            m_ViewportPos = { imagePos.x, imagePos.y };
            ImGui::Image((void*)texture->GetID(), regionAvail, ImVec2(0, 1), ImVec2(1, 0));
            ImGui::PopStyleVar();

            const Vector2& mousePos = Input::GetMousePosition();
            
            const ImVec2& windowPos = ImGui::GetWindowPos();
            const auto windowPosNuake = Vector2(windowPos.x, windowPos.y);
            const ImVec2& windowSize = ImGui::GetWindowSize();
            const bool isInsideWidth = mousePos.x > windowPos.x && mousePos.x < windowPos.x + windowSize.x;
            const bool isInsideHeight = mousePos.y > windowPos.y && mousePos.y < windowPos.y + windowSize.y;
            m_IsHoveringViewport = isInsideWidth && isInsideHeight;

            
            if (ImGui::BeginDragDropTarget())
            {
                auto& gbuffer = Engine::GetCurrentScene()->m_SceneRenderer->GetGBuffer();
                Vector2 textureSize = gbuffer.GetTexture(GL_DEPTH_ATTACHMENT)->GetSize();
                auto pixelPos = Input::GetMousePosition() - windowPosNuake;
                pixelPos.y = gbuffer.GetSize().y - pixelPos.y; // imgui coords are inverted on the Y axis

                auto gizmoBuffer = Engine::GetCurrentWindow()->GetFrameBuffer();
                gizmoBuffer->Bind();
                bool foundSomethingToSelect = false;
                Vector3 dragnDropWorldPos = Vector3(0, 0, 0);
                if (const float result = gizmoBuffer->ReadDepth(pixelPos); result > 0)
                {
                    const Matrix4& proj = Engine::GetCurrentScene()->m_EditorCamera->GetPerspective();
                    Matrix4 view = Engine::GetCurrentScene()->m_EditorCamera->GetTransform();
                    
                    pixelPos.y = (Input::GetMousePosition() - windowPosNuake).y;
                    dragnDropWorldPos = DepthToWorldPosition(pixelPos, result, proj, view, textureSize);

                    auto renderer = Engine::GetCurrentScene()->m_SceneRenderer;
                    const ImGuiPayload* payload = ImGui::GetDragDropPayload();
                    if(payload && payload->IsDataType("_Model"))
                    {
                        char* file = (char*)payload->Data;
                        std::string fullPath = std::string(file, 256);
                        fullPath = Nuake::FileSystem::AbsoluteToRelative(fullPath);

                        Matrix4 transform = Matrix4(1.0f);
                        transform = glm::translate(transform, dragnDropWorldPos);

                        if (!renderer->IsTempModelLoaded(fullPath))
                        {
                            auto loader = ModelLoader();
                            auto modelResource = loader.LoadModel(fullPath);

                            Matrix4 transform = Matrix4(1.0f);
                            transform = glm::translate(transform, dragnDropWorldPos);
                            renderer->DrawTemporaryModel(fullPath, modelResource, transform);
                        }
                        else
                        {
                            renderer->DrawTemporaryModel(fullPath, nullptr, transform);
                        }

                    }

                    // Engine::GetCurrentScene()->m_SceneRenderer->DrawDebugLine(dragnDropWorldPos, dragnDropWorldPos + Vector3{0, 5, 0}, Vector4(1, 0, 1, 1), -1.0f);
                }
                gizmoBuffer->Unbind();

                if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("_Model"))
                {
                    char* file = (char*)payload->Data;
                    std::string fullPath = std::string(file, 256);
                    fullPath = Nuake::FileSystem::AbsoluteToRelative(fullPath);

                    auto loader = ModelLoader();
                    auto modelResource = loader.LoadModel(fullPath);

                    auto entity = Engine::GetCurrentScene()->CreateEntity(FileSystem::GetFileNameFromPath(fullPath));
                    ModelComponent& modelComponent = entity.AddComponent<ModelComponent>();
                    modelComponent.ModelPath = fullPath;
                    modelComponent.ModelResource = modelResource;
                    entity.GetComponent<TransformComponent>().SetLocalPosition(dragnDropWorldPos);
                }
            }
            else
            {
                Engine::GetCurrentScene()->m_SceneRenderer->ClearTemporaryModels();
            }

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
                if (!Selection.Entity.IsValid())
                {
                    Selection = EditorSelection();
                }
                else
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
                        glm::value_ptr(transform), NULL,
                        UseSnapping ? &CurrentSnapping.x : NULL
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
            }

            if (ImGui::IsWindowHovered() && m_IsHoveringViewport && !m_IsViewportFocused && Input::IsMouseButtonPressed(GLFW_MOUSE_BUTTON_2))
            {
                ImGui::FocusWindow(ImGui::GetCurrentWindow());
            }

            m_IsViewportFocused = ImGui::IsWindowFocused();

            if (!Engine::IsPlayMode() && ImGui::GetIO().WantCaptureMouse && m_IsHoveringViewport && Input::IsMouseButtonPressed(GLFW_MOUSE_BUTTON_1) && !ImGuizmo::IsUsing() && m_IsViewportFocused)
            {
                const float resolutionScale = Engine::GetProject()->Settings.ResolutionScale;
                auto& gbuffer = Engine::GetCurrentScene()->m_SceneRenderer->GetGBuffer();
                auto pixelPos = (Input::GetMousePosition() - windowPosNuake) * resolutionScale;
                pixelPos.y = gbuffer.GetSize().y - pixelPos.y; // imgui coords are inverted on the Y axis

                auto gizmoBuffer = Engine::GetCurrentWindow()->GetFrameBuffer();
                gizmoBuffer->Bind();
                bool foundSomethingToSelect = false;
                if (const int result = gizmoBuffer->ReadPixel(1, pixelPos); result > 0)
                {
                    auto ent = Entity{ (entt::entity)(result - 1), Engine::GetCurrentScene().get() };
                    if (ent.IsValid())
                    {
                        Selection = EditorSelection(ent);
                        foundSomethingToSelect = true;
                    }
                }
                gizmoBuffer->Unbind();

                if(!foundSomethingToSelect)
                {
                    gbuffer.Bind();
                    if (const int result = gbuffer.ReadPixel(3, pixelPos); result > 0)
                    {
                        auto ent = Entity{ (entt::entity)(result - 1), Engine::GetCurrentScene().get() };
                        if (ent.IsValid())
                        {
                            Selection = EditorSelection(ent);
                            foundSomethingToSelect = true;
                        }
                    }
                    else
                    {
                        Selection = EditorSelection(); // None
                    }

                    gbuffer.Unbind();
                }

                if (foundSomethingToSelect = true)
                {
                    m_ShouldUnfoldEntityTree = true;
                }
            }
        }
        else
        {
            ImGui::PopStyleVar();
        }

        ImGui::End();

       
    }

    void EditorInterface::DrawStatusBar()
    {
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, { 8, 8 });
        ImGuiViewportP* viewport = (ImGuiViewportP*)(void*)ImGui::GetMainViewport();
        ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_MenuBar;
        float height = ImGui::GetFrameHeight();
        ImGui::PushStyleColor(ImGuiCol_MenuBarBg, { m_StatusBarColor.r, m_StatusBarColor.g, m_StatusBarColor.b, m_StatusBarColor.a });
        if (ImGui::BeginViewportSideBar("##MainStatusBar", viewport, ImGuiDir_Down, height, window_flags)) {
            if (ImGui::BeginMenuBar()) 
            {
                ImGui::Text(m_StatusMessage.c_str());

                ImGui::SameLine();

                const float remainingWidth = ImGui::GetContentRegionAvail().x;
                auto nuakeLogoTexture = TextureManager::Get()->GetTexture("Resources/Images/logo_white.png");
                auto nuakeSize = nuakeLogoTexture->GetSize();
                float sizeDiff = height / nuakeSize.y;
                float scale = 0.5f;
                const float logoWidth = nuakeSize.x * sizeDiff;
                
                std::string version = "dev";
#ifdef NK_DEBUG
                version += " - debug";
#endif
                ImVec2 textSize = ImGui::CalcTextSize(version.c_str());
                ImGui::Dummy({ remainingWidth - (logoWidth / 1.75f) - textSize.x - 8, height });

                ImGui::SameLine();
                ImGui::Text(version.c_str());
                ImGui::SameLine();

                ImGui::SetCursorPosY(height / 4.0 );
                ImGui::Image((ImTextureID)(nuakeLogoTexture->GetID()), ImVec2(logoWidth, height) * scale, ImVec2{0, 1}, ImVec2{1, 0});
                ImGui::EndMenuBar();
            }
            ImGui::End();
        }
        ImGui::PopStyleColor();
        ImGui::PopStyleVar();

    }

    void EditorInterface::DrawMenuBars()
    {
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, { 8, 8 });
        ImGuiViewportP* viewport = (ImGuiViewportP*)(void*)ImGui::GetMainViewport();
        ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_MenuBar;
        float height = ImGui::GetFrameHeight();
        if (ImGui::BeginViewportSideBar("##SecondaryMenuBar", viewport, ImGuiDir_Up, height, window_flags)) 
        {
            if (ImGui::BeginMenuBar()) 
            {
                float availWidth = ImGui::GetContentRegionAvail().x;
                float windowWidth = ImGui::GetWindowWidth();

                float used = windowWidth - availWidth;
                float half = windowWidth / 2.0;
                float needed = half - used;

                ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(2, 2));
                ImGui::PushStyleColor(ImGuiCol_Button, IM_COL32(0, 0, 0, 0));

                if (ImGui::Button(ICON_FA_ARROWS_ALT, ImVec2(30, 30)) || Input::IsKeyDown(Key::W))
                {
                    CurrentOperation = ImGuizmo::OPERATION::TRANSLATE;
                }
                ImGui::SameLine();
                if (ImGui::Button(ICON_FA_SYNC_ALT, ImVec2(30, 30)) || Input::IsKeyDown(Key::E))
                {
                    CurrentOperation = ImGuizmo::OPERATION::ROTATE;
                }
                ImGui::SameLine();
                if (ImGui::Button(ICON_FA_EXPAND_ALT, ImVec2(30, 30)) || Input::IsKeyDown(Key::R))
                {
                    CurrentOperation = ImGuizmo::OPERATION::SCALE;
                }
                ImGui::SameLine();
                ImGui::Dummy({ ImGui::GetContentRegionAvail().x / 2.0f - (76.0f / 2.0f), 8.0f });
                ImGui::SameLine();

                if (Engine::IsPlayMode() && Engine::GetTimeScale() != 0.0f)
                {
                    Color color = Engine::GetProject()->Settings.PrimaryColor;
                    ImGui::PushStyleColor(ImGuiCol_Text, { color.r, color.g, color.b, 1.0f });
                    if (ImGui::Button(ICON_FA_PAUSE, ImVec2(30, 30)) || (Input::IsKeyPressed(Key::F6)))
                    {
                        Engine::SetGameState(GameState::Paused);

                        SetStatusMessage("Paused");
                    }
                    ImGui::PopStyleColor();

                    if (ImGui::BeginItemTooltip())
                    {
                        ImGui::Text("Pause game (F6)");
                        ImGui::EndTooltip();
                    }
                }
                else
                {
                    bool playButtonPressed;
                    std::string tooltip;
                    if (Engine::GetGameState() == GameState::Paused)
                    {
                        Color color = Engine::GetProject()->Settings.PrimaryColor;
                        ImGui::PushStyleColor(ImGuiCol_Text, { color.r, color.g, color.b, 1.0f });
                        playButtonPressed = ImGui::Button(ICON_FA_PLAY, ImVec2(30, 30)) || Input::IsKeyPressed(Key::F6);
                        ImGui::PopStyleColor();

                        tooltip = "Resume (F6)";
                    }
                    else
                    {
                        playButtonPressed = ImGui::Button(ICON_FA_PLAY, ImVec2(30, 30)) || Input::IsKeyPressed(Key::F5);
                        tooltip = "Build & Play (F5)";

                    }
                   
                    if (playButtonPressed)
                    {
                        if (Engine::GetGameState() == GameState::Paused)
                        {
                            PushCommand(SetGameState(GameState::Playing));

                            Color color = Engine::GetProject()->Settings.PrimaryColor;
                            std::string statusMessage = ICON_FA_RUNNING + std::string(" Playing...");
                            SetStatusMessage(statusMessage.c_str(), { color.r, color.g, color.b, 1.0f });
                        }
                        else
                        {
                            this->SceneSnapshot = Engine::GetCurrentScene()->Copy();
                            
                            std::string statusMessage = ICON_FA_HAMMER + std::string("  Building .Net solution...");
                            SetStatusMessage(statusMessage);

                            auto job = [this]()
                            {
                                this->errors = ScriptingEngineNet::Get().BuildProjectAssembly(Engine::GetProject());
                            };

                            Selection = EditorSelection();

                            JobSystem::Get().Dispatch(job, [this]()
                            {
                                if (errors.size() > 0)
                                {
                                    SetStatusMessage("Failed to build scripts! See Logger for more info", { 1.0f, 0.1f, 0.1f, 1.0f });

                                    Logger::Log("Build FAILED.", ".net", CRITICAL);
                                    for (CompilationError error : errors)
                                    {
                                        const std::string errorMessage = error.file + "( line " + std::to_string(error.line) + "): " + error.message;
                                        Logger::Log(errorMessage, ".net", CRITICAL);
                                    }
                                }
                                else
                                {
                                    Engine::GetProject()->ExportEntitiesToTrenchbroom();

                                    SetStatusMessage("Entering play mode...");

                                    PushCommand(SetGameState(GameState::Playing));

                                    std::string statusMessage = ICON_FA_RUNNING + std::string(" Playing...");
                                    SetStatusMessage(statusMessage.c_str(), Engine::GetProject()->Settings.PrimaryColor);
                                }
                            });
                        }
                    }

                    if (ImGui::BeginItemTooltip())
                    {
                        ImGui::Text(tooltip.c_str());
                        ImGui::EndTooltip();
                    }
                }

                ImGui::SameLine();

                const bool wasPlayMode = Engine::GetGameState() != GameState::Stopped;
                if (!wasPlayMode)
                {
                    ImGui::BeginDisabled();
                }

                if ((ImGui::Button(ICON_FA_STOP, ImVec2(30, 30)) || Input::IsKeyPressed(Key::F5)) && wasPlayMode)
                {
                    Engine::ExitPlayMode();

                    Engine::SetCurrentScene(SceneSnapshot);
                    Selection = EditorSelection();
                    SetStatusMessage("Ready");
                }

                if (!wasPlayMode)
                {
                    ImGui::EndDisabled();
                }
                else
                {
                    if (ImGui::BeginItemTooltip())
                    {
                        ImGui::Text("Stop game (F5)");
                        ImGui::EndTooltip();
                    }
                }

                ImGui::SameLine();

                ImGui::PopStyleColor();

                float lineHeight = 130.0f;
                float separatorHeight = lineHeight * 8.0f;
                float separatorThickness = 1.0f;

                ImVec2 curPos = ImGui::GetCursorPos();
                ImVec2 min = ImVec2(curPos.x - separatorThickness, curPos.y - separatorHeight);
                ImVec2 max = ImVec2(curPos.x + separatorThickness, curPos.y - separatorHeight);
                ImGui::GetWindowDrawList()->AddRectFilled(min, max, IM_COL32(255, 255, 255, 32), 2.0f);

                ImGui::SameLine();

                const int sizeofRightPart = 176;

                ImGui::Dummy(ImVec2(ImGui::GetContentRegionAvail().x - sizeofRightPart, 30));

                ImGui::SameLine();

                const auto& io = ImGui::GetIO();
                float frameTime = 1000.0f / io.Framerate;
                int fps = (int)io.Framerate;
                ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 0.0f);
                ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.1f, 0.1f, 0.1f, 1));
                ImGui::BeginChild("FPS", ImVec2(70, 30), false);

                std::string text = std::to_string(fps) + " fps";
                ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x / 1.25f - ImGui::CalcTextSize(text.c_str()).x
                    - ImGui::GetScrollX() - 2.f * ImGui::GetStyle().ItemSpacing.x);
                ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 15 - (ImGui::CalcTextSize(text.c_str()).y) / 2.0);
                ImGui::Text(text.c_str());

                ImGui::EndChild();
                ImGui::SameLine();
                ImGui::BeginChild("frametime", ImVec2(70, 30), false);
                std::ostringstream out;
                out.precision(2);
                out << std::fixed << frameTime;
                text = out.str() + " ms";
                ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x / 1.25 - ImGui::CalcTextSize(text.c_str()).x
                    - ImGui::GetScrollX() - 2 * ImGui::GetStyle().ItemSpacing.x);
                ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 15 - (ImGui::CalcTextSize(text.c_str()).y) / 2.0);
                ImGui::Text(text.c_str());

                ImGui::EndChild();

                ImGui::SameLine();

                ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(2, 2));
                ImGui::PushStyleColor(ImGuiCol_Button, IM_COL32(0, 0, 0, 0));

                if (ImGui::Button(ICON_FA_HAMMER, ImVec2(30, 30)))
                {
                    SetStatusMessage(std::string(ICON_FA_HAMMER)+ " Building solution...", { 0.1f, 0.1f, 1.0f, 1.0f });

                    auto job = [this]()
                        {
                            this->errors = ScriptingEngineNet::Get().BuildProjectAssembly(Engine::GetProject());
                        };

                    JobSystem::Get().Dispatch(job, [this]()
                    {
                        if (errors.size() > 0)
                        {
                            SetStatusMessage("Failed to build scripts! See Logger for more info", { 1.0f, 0.1f, 0.1f, 1.0f });

                            Logger::Log("Build FAILED.", ".net", CRITICAL);
                            for (CompilationError error : errors)
                            {
                                const std::string errorMessage = error.file + "( line " + std::to_string(error.line) + "): " + error.message;
                                Logger::Log(errorMessage, ".net", CRITICAL);
                            }
                        }
                        else
                        {
                            Engine::GetProject()->ExportEntitiesToTrenchbroom();
                            SetStatusMessage("Build succesful!");
                        }
                    });
                }

                if (ImGui::BeginItemTooltip())
                {
                    ImGui::Text("Built .Net project");
                    ImGui::EndTooltip();
                }

                ImGui::PopStyleColor();
                ImGui::PopStyleVar();

                ImGui::PopStyleColor();
                ImGui::PopStyleVar();

                ImGui::PopStyleVar(1);
                ImGui::EndMenuBar();
            }
            ImGui::End();
        }
        ImGui::PopStyleVar();
    }

    static int selected = 0;
    Entity QueueDeletion;
    void EditorInterface::DrawEntityTree(Entity e)
    {
        ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2{ 0.0f, 0.0f });

        ImGuiTreeNodeFlags base_flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_FramePadding | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_SpanAllColumns;

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
        else if (e.HasComponent<BSPBrushComponent>())
        {
            ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 255, 255, 120));
        }

        if (!m_IsRenaming && m_ShouldUnfoldEntityTree && Selection.Type == EditorSelectionType::Entity && e.GetScene()->EntityIsParent(Selection.Entity, e))
        {
            ImGui::SetNextItemOpen(true);
        }

        auto cursorPos = ImGui::GetCursorPos();
        ImGui::SetNextItemAllowOverlap();
        bool open = ImGui::TreeNodeEx(name.c_str(), base_flags);

        if (m_IsRenaming)
        {
            if (Selection.Type == EditorSelectionType::Entity && Selection.Entity == e)
            {
                ImGui::SetCursorPosY(cursorPos.y);
                ImGui::Indent();
                ImGui::InputText("##renamingEntity", &name);
                ImGui::Unindent();
                if (Input::IsKeyDown(Key::ENTER))
                {
                    nameComponent.Name = name;
                    m_IsRenaming = false;
                }
            }
        }

        bool isDragging = false;
        if (nameComponent.IsPrefab && e.HasComponent<PrefabComponent>() || e.HasComponent<BSPBrushComponent>())
        {
			ImGui::PopStyleColor();
        }
		else if (!m_IsRenaming && ImGui::BeginDragDropSource())
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

        if (!isDragging && ImGui::IsItemHovered() && ImGui::IsMouseReleased(0))
        {
            // We selected another another that we werent renaming
            if (Selection.Entity != e)
            {
                m_IsRenaming = false;
            }

            Selection = EditorSelection(e);
        }

        if (!isDragging && (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0)) || Input::IsKeyPressed(Key::F2))
        {
            m_IsRenaming = true;
        }

        if (!m_IsRenaming && Selection.Type == EditorSelectionType::Entity && Input::IsKeyPressed(Key::DELETE_KEY))
        {
            QueueDeletion = Selection.Entity;
        }

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
            bool hasScript = e.HasComponent<NetScriptComponent>();
            if (hasScript)
            {
                std::string scrollIcon = std::string(ICON_FA_SCROLL) + "##" + name;
                ImGui::PushStyleColor(ImGuiCol_Button, { 0, 0, 0, 0 });
                if (ImGui::Button(scrollIcon.c_str(), { 40, 0 }))
                {
                    auto& scriptComponent = e.GetComponent<NetScriptComponent>();
                    if (!scriptComponent.ScriptPath.empty() && FileSystem::FileExists(scriptComponent.ScriptPath))
                    {
                        OS::OpenIn(FileSystem::RelativeToAbsolute(scriptComponent.ScriptPath));
                    }
                }
                ImGui::PopStyleColor();
            }
        }

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

            BEGIN_COLLAPSE_HEADER(BLOOM)
                if (ImGui::BeginTable("BloomTable", 3, ImGuiTableFlags_BordersInner | ImGuiTableFlags_SizingStretchProp))
                {
                    ImGui::TableSetupColumn("name", 0, 0.3f);
                    ImGui::TableSetupColumn("set", 0, 0.6f);
                    ImGui::TableSetupColumn("reset", 0, 0.1f);

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
                    ImGui::TableSetupColumn("name", 0, 0.3f);
                    ImGui::TableSetupColumn("set", 0, 0.6f);
                    ImGui::TableSetupColumn("reset", 0, 0.1f);

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
                        ImGui::Text("Volumetric Strength");
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

                    SSR* ssr = env->mSSR.get();
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

                    {
                        ImGui::TableNextColumn();
                        // Title
                        ImGui::Text("Focal Depth");
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
                        ImGui::Text("Focal Length");
                        ImGui::TableNextColumn();

                        ImGui::DragFloat("##doffocalLength", &env->DOFFocalLength);
                        ImGui::TableNextColumn();

                        // Reset button
                        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1, 1, 1, 0));
                        std::string resetDOFFocalLength = ICON_FA_UNDO + std::string("##resetDOFFocalLength");
                        if (ImGui::Button(resetDOFFocalLength.c_str())) env->DOFFocalLength = 1.0f;
                        ImGui::PopStyleColor();
                    }

                    {
                        ImGui::TableNextColumn();
                        // Title
                        ImGui::Text("fStop");
                        ImGui::TableNextColumn();

                        ImGui::DragFloat("##doffstop", &env->DOFFstop);
                        ImGui::TableNextColumn();

                        // Reset button
                        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1, 1, 1, 0));
                        std::string resetDOFFstop = ICON_FA_UNDO + std::string("##resetDOFFstop");
                        if (ImGui::Button(resetDOFFstop.c_str())) env->DOFFstop = 1.0f;
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
                        ImGui::Text("Display auto focus");
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
                        ImGui::Text("Manual focus");
                        ImGui::TableNextColumn();

                        ImGui::Checkbox("##dofmanualfocus", &env->DOFManualFocus);
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
                        ImGui::Text("fStop");
                        ImGui::TableNextColumn();

                        ImGui::DragFloat("##doffstop", &env->DOFFstop);
                        ImGui::TableNextColumn();

                        // Reset button
                        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1, 1, 1, 0));
                        std::string resetDOFFstop = ICON_FA_UNDO + std::string("##resetDOFFstop");
                        if (ImGui::Button(resetDOFFstop.c_str())) env->DOFFstop = 1.0f;
                        ImGui::PopStyleColor();
                    }

                    {
                        ImGui::TableNextColumn();
                        // Title
                        ImGui::Text("DOF Start");
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
                        ImGui::Text("DOF Distance");
                        ImGui::TableNextColumn();

                        ImGui::DragFloat("##dofdistance", &env->DOFDist);
                        ImGui::TableNextColumn();

                        // Reset button
                        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1, 1, 1, 0));
                        std::string resetDOFDist = ICON_FA_UNDO + std::string("##resetDOFDist");
                        if (ImGui::Button(resetDOFDist.c_str())) env->DOFDist = 1.0f;
                        ImGui::PopStyleColor();
                    }

                    {
                        ImGui::TableNextColumn();
                        // Title
                        ImGui::Text("DOF Threshold");
                        ImGui::TableNextColumn();

                        ImGui::DragFloat("##dofthreshold", &env->DOFThreshold, 0.001f, 0.0f);
                        ImGui::TableNextColumn();

                        // Reset button
                        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1, 1, 1, 0));
                        std::string resetDOFDist = ICON_FA_UNDO + std::string("##resetDOFDist");
                        if (ImGui::Button(resetDOFDist.c_str())) env->DOFDist = 1.0f;
                        ImGui::PopStyleColor();
                    }

                    {
                        ImGui::TableNextColumn();
                        // Title
                        ImGui::Text("DOF Feather");
                        ImGui::TableNextColumn();

                        ImGui::DragFloat("##doffeather", &env->DOFFeather, 0.001f, 0.0f, 1.0f);
                        ImGui::TableNextColumn();

                        // Reset button
                        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1, 1, 1, 0));
                        std::string resetDOFDist = ICON_FA_UNDO + std::string("##resetdoffeather");
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

        std::string title = ICON_FA_TREE + std::string("   Hierarchy");
        if (ImGui::Begin(title.c_str()))
        {
            std::string searchQuery = "";
            
            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, { 8, 8 });
            ImGui::InputTextWithHint("##search", "Search entity", &searchQuery, 0, 0, 0);
            ImGui::PopStyleVar();

            ImGui::SameLine();

            if (UI::PrimaryButton("Add Entity", { ImGui::GetContentRegionAvail().x, 0 }))
            {
                ImGui::OpenPopup("create_entity_popup");
            }

            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 8, 8 });

            if (ImGui::BeginPopup("create_entity_new_popup"))
            {
                ImGui::BeginChild("entity_child", ImVec2(442, 442), ImGuiChildFlags_AlwaysUseWindowPadding);

                ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
                ImGui::InputTextWithHint("##search", "Search entity", &searchQuery, 0, 0, 0);
                ImGui::PopItemWidth();

                //ImGui::Button("Empty", { 130, 130 });
                auto textureManager = TextureManager::Get();
                ImVec2 buttonSize = { 120, 120 };
                ImGui::ImageButton((ImTextureID)textureManager->GetTexture("Resources/Images/cube.png")->GetID(), buttonSize, ImVec2(0, 1), ImVec2(1, 0));
                ImGui::SameLine();
                ImGui::ImageButton((ImTextureID)textureManager->GetTexture("Resources/Images/cube.png")->GetID(), buttonSize, ImVec2(0, 1), ImVec2(1, 0));
                ImGui::SameLine();
                ImGui::ImageButton((ImTextureID)textureManager->GetTexture("Resources/Images/light.png")->GetID(), buttonSize, ImVec2(0, 1), ImVec2(1, 0));

                ImGui::ImageButton((ImTextureID)textureManager->GetTexture("Resources/Images/physics.png")->GetID(), buttonSize, ImVec2(0, 1), ImVec2(1, 0));
                ImGui::SameLine();
                ImGui::ImageButton((ImTextureID)textureManager->GetTexture("Resources/Images/shape.png")->GetID(), buttonSize, ImVec2(0, 1), ImVec2(1, 0));
                ImGui::SameLine();
                ImGui::ImageButton((ImTextureID)textureManager->GetTexture("Resources/Images/code.png")->GetID(), buttonSize, ImVec2(0, 1), ImVec2(1, 0));

                ImGui::ImageButton((ImTextureID)textureManager->GetTexture("Resources/Images/box.png")->GetID(), buttonSize, ImVec2(0, 1), ImVec2(1, 0));
                ImGui::SameLine();
                ImGui::ImageButton((ImTextureID)textureManager->GetTexture("Resources/Images/wrench.png")->GetID(), buttonSize, ImVec2(0, 1), ImVec2(1, 0));
                ImGui::SameLine();

                ImGui::BeginDisabled();
                ImGui::Button("Addons", { 130, 130 });
                ImGui::EndDisabled();

                ImGui::EndChild();
                ImGui::EndPopup();
            }
            ImGui::PopStyleVar();


            if (ImGui::BeginPopup("create_entity_popup"))
            {
                Entity entity;
                Ref<Scene> scene = Engine::GetCurrentScene();
                if (ImGui::MenuItem("Empty"))
                {
                    entity = scene->CreateEntity("Empty");
                }

                if(ImGui::BeginMenu("3D"))
                {
                    if (ImGui::MenuItem("Camera"))
                    {
                        entity = scene->CreateEntity("Camera");
                        entity.AddComponent<CameraComponent>();
                    }
                    if (ImGui::MenuItem("Model"))
                    {
                        entity = scene->CreateEntity("Model");
                        entity.AddComponent<ModelComponent>();
                    }
                    if (ImGui::MenuItem("Skinned Model"))
                    {
                        entity = scene->CreateEntity("Skinned Model");
                        entity.AddComponent<SkinnedModelComponent>();
                    }
                    if (ImGui::MenuItem("Sprite"))
                    {
                        entity = scene->CreateEntity("Sprite");
                        entity.AddComponent<SpriteComponent>();
                    }
                    if (ImGui::MenuItem("Particle Emitter"))
                    {
                        entity = scene->CreateEntity("Particle Emitter");
                        entity.AddComponent<ParticleEmitterComponent>();
                    }
                    if (ImGui::MenuItem("Light"))
                    {
                        entity = scene->CreateEntity("Light");
                        entity.AddComponent<LightComponent>();
                    }
                    if (ImGui::MenuItem("Quake Map"))
                    {
                        entity = scene->CreateEntity("Quake Map");
                        entity.AddComponent<QuakeMapComponent>();
                    }
                    if (ImGui::MenuItem("NavMesh Volume"))
                    {
                        entity = scene->CreateEntity("NavMesh Volume");
                        entity.AddComponent<NavMeshVolumeComponent>();
                    }
                    ImGui::EndMenu();
                }

                if (ImGui::BeginMenu("Physics"))
                {
                    if (ImGui::MenuItem("Character Controller"))
                    {
                        entity = scene->CreateEntity("Character Controller");
                        entity.AddComponent<CharacterControllerComponent>();
                    }
                    if (ImGui::MenuItem("Rigid Body"))
                    {
                        entity = scene->CreateEntity("Rigid Body");
                        entity.AddComponent<RigidBodyComponent>();
                    }
                    ImGui::EndMenu();
                }
                
                if (ImGui::BeginMenu("Colliders"))
                {
                    if (ImGui::MenuItem("Box Collider")) 
                    {
                        entity = scene->CreateEntity("Box Collider");
                        entity.AddComponent<BoxColliderComponent>();
                    }
                    if (ImGui::MenuItem("Sphere Collider")) 
                    {
                        entity = scene->CreateEntity("Sphere Collider");
                        entity.AddComponent<SphereColliderComponent>();
                    }
                    if (ImGui::MenuItem("Capsule Collider")) 
                    {
                        entity = scene->CreateEntity("Capsule Collider");
                        entity.AddComponent<CapsuleColliderComponent>();
                    }
                    if (ImGui::MenuItem("Cylinder Collider")) 
                    {
                        entity = scene->CreateEntity("Cylinder Collider");
                        entity.AddComponent<CylinderColliderComponent>();
                    }
                    if (ImGui::MenuItem("Mesh Collider")) 
                    {
                        entity = scene->CreateEntity("Mesh Collider");
                        entity.AddComponent<MeshColliderComponent>();
                    }
                    ImGui::EndMenu();
                }

                if (ImGui::BeginMenu("Audio"))
                {
                    if (ImGui::MenuItem("Audio Emitter"))
                    {
                        entity = scene->CreateEntity("Audio Emitter");
                        entity.AddComponent<AudioEmitterComponent>();
                    }
                    ImGui::EndMenu();
                }

                if (ImGui::BeginMenu("Script"))
                {
                    if (ImGui::MenuItem("Script"))
                    {
                        entity = scene->CreateEntity("Script");
                        entity.AddComponent<WrenScriptComponent>();
                    }
                    ImGui::EndMenu();
                }
                
                if (entity.IsValid())
                {
                    if (Selection.Type == EditorSelectionType::Entity && Selection.Entity.IsValid())
                    {
                        Selection.Entity.AddChild(entity);
                    }
                    else
                    {
                        auto& camera = Engine::GetCurrentScene()->m_EditorCamera;
                        Vector3 newEntityPos = camera->Translation + camera->Direction;
                        entity.GetComponent<TransformComponent>().SetLocalPosition(newEntityPos);
                    }

                    Selection = EditorSelection(entity);
                }

                ImGui::EndPopup();
            }

            // Draw a tree of entities.
            ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(26.f / 255.0f, 26.f / 255.0f, 26.f / 255.0f, 1));
            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, { 4, 4 });
            if (ImGui::BeginChild("Scene tree", ImGui::GetContentRegionAvail(), false))
            {
                if (ImGui::BeginTable("entity_table", 4, ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_SizingStretchProp))
                {
                    ImGui::TableSetupColumn("   Label", ImGuiTableColumnFlags_IndentEnable | ImGuiTableColumnFlags_NoResize | ImGuiTableColumnFlags_WidthStretch);
                    ImGui::TableSetupColumn("Type", ImGuiTableColumnFlags_IndentDisable | ImGuiTableColumnFlags_WidthFixed);
                    ImGui::TableSetupColumn("Script", ImGuiTableColumnFlags_NoResize | ImGuiTableColumnFlags_IndentDisable | ImGuiTableColumnFlags_WidthFixed);
                    ImGui::TableSetupColumn("Visibility   ", ImGuiTableColumnFlags_NoResize | ImGuiTableColumnFlags_IndentDisable | ImGuiTableColumnFlags_WidthFixed);
                    ImGui::TableHeadersRow();

                    ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(0, 0));
                    std::vector<Entity> entities = scene->GetAllEntities();
                    for (Entity e : entities)
                    {
                        ImGuiTreeNodeFlags base_flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_SpanFullWidth;
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
				
            }
            ImGui::EndChild();
            ImGui::PopStyleVar();
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
            if (ImGui::Button("Clear Logs"))
            {
                Logger::ClearLogs();
                SetStatusMessage("Logs cleared.");
            }

            ImGui::SameLine();
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

                    ImVec4 redColor = ImVec4(0.6, 0.1f, 0.1f, 0.2f);
                    ImVec4 yellowColor = ImVec4(0.6, 0.6f, 0.1f, 0.2f);
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

                    if (l.type == CRITICAL)
                    {
                        ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, ImGui::GetColorU32(redColor), -1);
                    }
                    else if (l.type == WARNING)
                    {
                        ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, ImGui::GetColorU32(yellowColor), -1);
                    }
                    else
                    {
                        ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, ImGui::GetColorU32(ImVec4(1, 1, 1, 0.0)), -1);
                    }

                    std::string displayMessage = l.message; 
                    if (l.count > 0)
                    {
                        displayMessage += "(" + std::to_string(l.count) + ")";
                    } 

                    ImGui::TextWrapped(displayMessage.c_str());
                    ImGui::PopStyleColor();

                    ImGui::TableNextColumn();
                }
                ImGui::PopStyleVar();

                if (AutoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
                {
                    ImGui::SetScrollHereY(1.0f);
                }

                ImGui::EndTable();
            }
        }
        ImGui::End();
    }

    void EditorInterface::DrawProjectSettings()
    {
        static int settingCategoryIndex = 0;
        if (ImGui::Begin("Project Settings", &m_ShowProjectSettings, ImGuiWindowFlags_NoDocking))
        {
            ImVec4* colors = ImGui::GetStyle().Colors;
            ImGui::PushStyleColor(ImGuiCol_ChildBg, colors[ImGuiCol_TitleBgCollapsed]);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 8);
            ImGui::BeginChild("ProjectSettingsLeft", { 200, ImGui::GetContentRegionAvail().y }, true);
            {
                ImGuiTreeNodeFlags base_flags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_SpanFullWidth | ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_FramePadding;

                bool is_selected = settingCategoryIndex == 1;
                if (is_selected)
                    base_flags |= ImGuiTreeNodeFlags_Selected;

                ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(0, 0));
                ImGui::PushFont(FontManager::GetFont(Bold));
                ImGui::TreeNodeEx("General", base_flags);
                if (ImGui::IsItemClicked())
                {
                    settingCategoryIndex = 0;
                }

                ImGui::TreePop();

                ImGui::TreeNodeEx("Viewport", base_flags);
                if (ImGui::IsItemClicked())
                {
                    settingCategoryIndex = 1;
                }

                ImGui::TreePop();
                ImGui::TreeNodeEx("Rendering", base_flags);
                if (ImGui::IsItemClicked())
                {
                    settingCategoryIndex = 2;
                }

                ImGui::TreePop();
                ImGui::TreeNodeEx("Audio", base_flags);
                if (ImGui::IsItemClicked())
                {
                    settingCategoryIndex = 3;
                }

                ImGui::TreePop();
                ImGui::TreeNodeEx("C#", base_flags);
                if (ImGui::IsItemClicked())
                {
                    settingCategoryIndex = 4;
                }

                ImGui::TreePop();
                ImGui::TreeNodeEx("Trenchbroom", base_flags);
                if (ImGui::IsItemClicked())
                {
                    settingCategoryIndex = 5;
                }

                ImGui::TreePop();
                ImGui::TreeNodeEx("Plugins", base_flags);
                if (ImGui::IsItemClicked())
                {
                    settingCategoryIndex = 6;
                }

                ImGui::PopStyleVar();
                ImGui::TreePop();
                ImGui::PopFont();
            }
            ImGui::PopStyleVar();
            ImGui::PopStyleColor();

            ImGui::EndChild();

            ImGui::SameLine();

            ImGui::BeginChild("ProjectSettingsRight", ImGui::GetContentRegionAvail(), ImGuiChildFlags_Border);
            {
                ImGui::Text("Right side");
            }
            ImGui::EndChild();
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

        window_flags |= ImGuiWindowFlags_NoMove;
        ImGuiViewport* viewport = ImGui::GetWindowViewport();
        ImVec2 work_area_pos = ImGui::GetCurrentWindow()->Pos;   // Instead of using viewport->Pos we use GetWorkPos() to avoid menu bars, if any!
        ImVec2 work_area_size = ImGui::GetCurrentWindow()->Size;
        ImVec2 window_pos = ImVec2((corner & 1) ? (work_area_pos.x + work_area_size.x - DISTANCE) : (work_area_pos.x + DISTANCE), (corner & 2) ? (work_area_pos.y + work_area_size.y - DISTANCE) : (work_area_pos.y + DISTANCE));
        ImVec2 window_pos_pivot = ImVec2((corner & 1) ? 1.0f : 0.0f, (corner & 2) ? 1.0f : 0.0f);
        ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, window_pos_pivot);
        ImGui::SetNextWindowViewport(viewport->ID);

        ImGui::SetNextWindowBgAlpha(0.35f); // Transparent background
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 32.0f);
        if (ImGui::Begin("ActionBar", &m_ShowOverlay, window_flags))
        {
            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(2, 2));
            ImGui::PushStyleColor(ImGuiCol_Button, IM_COL32(0, 0, 0, 0));
            ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 100);
            
            bool selectedMode = CurrentOperation == ImGuizmo::OPERATION::TRANSLATE;
            if (selectedMode)
            {
                Color color = Engine::GetProject()->Settings.PrimaryColor;
                ImGui::PushStyleColor(ImGuiCol_Button, { color.r, color.g, color.b, 1.0f });
            }

            if (ImGui::Button(ICON_FA_ARROWS_ALT, ImVec2(30, 28)) || Input::IsKeyDown(Key::W))
            {
                CurrentOperation = ImGuizmo::OPERATION::TRANSLATE;
            }

            
            UI::Tooltip("Translate");
            if (selectedMode)
            {
                ImGui::PopStyleColor();
            }

            ImGui::SameLine();

            selectedMode = CurrentOperation == ImGuizmo::OPERATION::ROTATE;
            if (selectedMode)
            {
                Color color = Engine::GetProject()->Settings.PrimaryColor;
                ImGui::PushStyleColor(ImGuiCol_Button, { color.r, color.g, color.b, 1.0f });
            }

            if (ImGui::Button(ICON_FA_SYNC_ALT, ImVec2(30, 28)) || Input::IsKeyDown(Key::E))
            {
                CurrentOperation = ImGuizmo::OPERATION::ROTATE;
            }

            UI::Tooltip("Rotate");

            if (selectedMode)
            {
                ImGui::PopStyleColor();
            }

            ImGui::SameLine();

            selectedMode = CurrentOperation == ImGuizmo::OPERATION::SCALE;
            if (selectedMode)
            {
                Color color = Engine::GetProject()->Settings.PrimaryColor;
                ImGui::PushStyleColor(ImGuiCol_Button, { color.r, color.g, color.b, 1.0f });
            }

            if (ImGui::Button(ICON_FA_EXPAND_ALT, ImVec2(30, 28)) || Input::IsKeyDown(Key::R))
            {
                CurrentOperation = ImGuizmo::OPERATION::SCALE;
            }

            UI::Tooltip("Scale");

            if (selectedMode)
            {
                ImGui::PopStyleColor();
            }

            ImGui::SameLine();
            
            selectedMode = CurrentMode == ImGuizmo::MODE::WORLD;
            if (selectedMode)
            {
                Color color = Engine::GetProject()->Settings.PrimaryColor;
                ImGui::PushStyleColor(ImGuiCol_Button, { color.r, color.g, color.b, 1.0f });
            }

            if (ImGui::Button(ICON_FA_GLOBE, ImVec2(30, 28)))
            {
                CurrentMode = ImGuizmo::MODE::WORLD;
            }

            UI::Tooltip("Global Transformation");

            if (selectedMode)
            {
                ImGui::PopStyleColor();
            }

            ImGui::SameLine();

            selectedMode = CurrentMode == ImGuizmo::MODE::LOCAL;
            if (selectedMode)
            {
                Color color = Engine::GetProject()->Settings.PrimaryColor;
                ImGui::PushStyleColor(ImGuiCol_Button, { color.r, color.g, color.b, 1.0f });
            }

            if (ImGui::Button(ICON_FA_CUBE, ImVec2(30, 28)))
            {
                CurrentMode = ImGuizmo::MODE::LOCAL;
            }

            UI::Tooltip("Local Transformation");

            if (selectedMode)
            {
                ImGui::PopStyleColor();
            }

            ImGui::SameLine();

            ImGui::SameLine();
            ImGui::PushItemWidth(75);
            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, { 6, 6 });
            ImGui::DragFloat("##snapping", &CurrentSnapping.x, 0.01f, 0.0f, 100.0f);
            ImGui::PopStyleVar();

            ImGui::PopItemWidth();
            UI::Tooltip("Snapping");

            ImGui::PopStyleVar();
            ImGui::PopStyleVar();
            ImGui::PopStyleColor();
        }

        ImGui::PopStyleVar();
        ImGui::End();

		corner = 1;
		window_flags |= ImGuiWindowFlags_NoMove;
		viewport = ImGui::GetWindowViewport();
		work_area_pos = ImGui::GetCurrentWindow()->Pos;   // Instead of using viewport->Pos we use GetWorkPos() to avoid menu bars, if any!
		work_area_size = ImGui::GetCurrentWindow()->Size;
		window_pos = ImVec2((corner & 1) ? (work_area_pos.x + work_area_size.x - DISTANCE) : (work_area_pos.x + DISTANCE), (corner & 2) ? (work_area_pos.y + work_area_size.y - DISTANCE) : (work_area_pos.y + DISTANCE));
		window_pos_pivot = ImVec2((corner & 1) ? 1.0f : 0.0f, (corner & 2) ? 1.0f : 0.0f);
		ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, window_pos_pivot);
		ImGui::SetNextWindowViewport(viewport->ID);

		ImGui::SetNextWindowBgAlpha(0.35f); // Transparent background
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 32.0f);
		if (ImGui::Begin("GraphicsBar", &m_ShowOverlay, window_flags))
		{
			ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(2, 2));
			ImGui::PushStyleColor(ImGuiCol_Button, IM_COL32(0, 0, 0, 0));
			ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 100);
            ImGui::PushStyleColor(ImGuiCol_FrameBg, IM_COL32(20, 20, 20, 0));
            ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, IM_COL32(20, 20, 20, 60));
            ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, IM_COL32(33, 33, 33, 45));
            const char* items[] = { "Shaded", "Albedo", "Normal", "Depth"};
            ImGui::SetNextItemWidth(128);
			if (ImGui::BeginCombo("##Output", items[SelectedViewport]))
			{
				// Loop through each item and create a selectable item
				for (int i = 0; i < IM_ARRAYSIZE(items); i++)
				{
					bool is_selected = (SelectedViewport == i);  // Check if the current item is selected
					if (ImGui::Selectable(items[i], is_selected))
					{
                        SelectedViewport = i;  // Update the selected item
					}

					// Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
					if (is_selected)
						ImGui::SetItemDefaultFocus();
				}
				ImGui::EndCombo();
			}

			UI::Tooltip("Output");

            ImGui::PopStyleVar(2);
            ImGui::PopStyleColor(4);
		}

		ImGui::PopStyleVar();
		ImGui::End();

        int corner2 = 1;
        work_area_pos = ImGui::GetCurrentWindow()->Pos;   // Instead of using viewport->Pos we use GetWorkPos() to avoid menu bars, if any!
        work_area_size = ImGui::GetCurrentWindow()->Size;
        window_pos = ImVec2((corner2 & 1) ? (work_area_pos.x + work_area_size.x - DISTANCE) : (work_area_pos.x + DISTANCE), (corner2 & 2) ? (work_area_pos.y + work_area_size.y - DISTANCE) : (work_area_pos.y + DISTANCE));
        window_pos_pivot = ImVec2((corner2 & 1) ? 1.0f : 0.0f, (corner2 & 2) ? 1.0f : 0.0f);
        ImGui::SetNextWindowPos(window_pos + ImVec2(0, 40), ImGuiCond_Always, window_pos_pivot);
        ImGui::SetNextWindowViewport(viewport->ID);
        ImGui::SetNextWindowBgAlpha(0.35f); // Transparent background
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 32.0f);
        ImGui::SetNextWindowSize(ImVec2(16, ImGui::GetContentRegionAvail().y - DISTANCE * 2.0 - 40.0));
        if (ImGui::Begin("Controls", &m_ShowOverlay, window_flags))
        {
            const auto& editorCam = Engine::GetCurrentScene()->m_EditorCamera;
            const float camSpeed = editorCam->Speed;

            const float maxSpeed = 50.0f;
            const float minSpeed = 0.1f;
            const float normalizedSpeed = glm::clamp(camSpeed / maxSpeed, 0.0f, 1.0f);

            ImVec2 start = ImGui::GetWindowPos() - ImVec2(0.0, 4.0) ;
            ImVec2 end = start + ImGui::GetWindowSize() - ImVec2(0, 16.0);
            ImVec2 startOffset = ImVec2(start.x , end.y - (normalizedSpeed * (ImGui::GetWindowHeight() - 20.0)));

            ImGui::GetWindowDrawList()->AddRectFilled(startOffset + ImVec2(0, 10.0), end + ImVec2(0.0, 20.0), IM_COL32(255, 255, 255, 180), 8.0f, ImDrawFlags_RoundCornersAll);
            ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 100);
            ImGui::PopStyleVar();
        }

        ImGui::PopStyleVar();
        ImGui::End();

        corner = 2;

        if (Selection.Type == EditorSelectionType::Entity && Selection.Entity.IsValid() && Selection.Entity.HasComponent<CameraComponent>() && !Engine::IsPlayMode())
        {
            window_flags |= ImGuiWindowFlags_NoMove;
            viewport = ImGui::GetWindowViewport();
            work_area_pos = ImGui::GetCurrentWindow()->Pos;   // Instead of using viewport->Pos we use GetWorkPos() to avoid menu bars, if any!
            work_area_size = ImGui::GetCurrentWindow()->Size;
            window_pos = ImVec2((corner & 1) ? (work_area_pos.x + work_area_size.x - DISTANCE) : (work_area_pos.x + DISTANCE), (corner & 2) ? (work_area_pos.y + work_area_size.y - DISTANCE) : (work_area_pos.y + DISTANCE));
            window_pos_pivot = ImVec2((corner & 1) ? 1.0f : 0.0f, (corner & 2) ? 1.0f : 0.0f);
            ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, window_pos_pivot);
            ImGui::SetNextWindowViewport(viewport->ID);

            ImGui::SetNextWindowBgAlpha(0.35f); // Transparent background
            ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 32.0f);
            if (ImGui::Begin("VirtualViewport", &m_ShowOverlay, window_flags))
            {
                ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(2, 2));
                ImGui::PushStyleColor(ImGuiCol_Button, IM_COL32(0, 0, 0, 0));
                ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 100);
                ImGui::PushStyleColor(ImGuiCol_FrameBg, IM_COL32(20, 20, 20, 0));
                ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, IM_COL32(20, 20, 20, 60));
                ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, IM_COL32(33, 33, 33, 45));
                
                CameraComponent& camera = Selection.Entity.GetComponent<CameraComponent>();
                Ref<Camera> cam = camera.CameraInstance;
                auto& transform = Selection.Entity.GetComponent<TransformComponent>();

                const Quat& globalRotation = transform.GetGlobalRotation();
                const Matrix4& translationMatrix = glm::translate(Matrix4(1.0f), transform.GetGlobalPosition());
                const Matrix4& rotationMatrix = glm::mat4_cast(globalRotation);
                const Vector4& forward = Vector4(0, 0, -1, 1);
                const Vector4& globalForward = rotationMatrix * forward;

                const Vector4& right = Vector4(1, 0, 0, 1);
                const Vector4& globalRight = rotationMatrix * right;
                cam->Direction = globalForward;
                cam->Right = globalRight;
                cam->Translation = transform.GetGlobalPosition();
                cam->SetTransform(glm::inverse(transform.GetGlobalTransform()));

                auto sceneRenderer = Engine::GetCurrentScene()->m_SceneRenderer;
                sceneRenderer->BeginRenderScene(cam->GetPerspective(), cam->GetTransform(), cam->Translation);
                sceneRenderer->RenderScene(*Engine::GetCurrentScene().get(), *virtualCamera.get(), false);

                virtualCamera->Clear();
                ImGui::Image((void*)virtualCamera->GetTexture()->GetID(), { 640, 360 }, { 0, 1 }, {1, 0});

                ImGui::PopStyleVar(2);
                ImGui::PopStyleColor(4);
            }

            ImGui::PopStyleVar();
            ImGui::End();
        }
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

        finalPath += "/" + fileName;
        
        Ref<Project> project = Project::New(String::Split(fileName, '.')[0], "no description", finalPath);
        Engine::LoadProject(project);
        Engine::SetCurrentScene(Scene::New());

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
        if (!project->Deserialize(json::parse(FileSystem::ReadFile(projectPath, true))))
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
        if (projectPath.empty() || !FileSystem::FileExists(projectPath, true))
        {
            return;
        }

        Ref<Scene> scene = Scene::New();
        const std::string& fileContent = FileSystem::ReadFile(projectPath, true);
        if (!scene->Deserialize(json::parse(fileContent)))
        {
            Logger::Log("Error failed loading scene: " + projectPath, "editor", CRITICAL);
            return;
        }

        scene->Path = FileSystem::AbsoluteToRelative(projectPath);
        Engine::SetCurrentScene(scene);
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
                if (ImGui::MenuItem("Save Project", "CTRL+S"))
                {
                    PushCommand(SaveProjectCommand(Engine::GetProject()));

                    Selection = EditorSelection();

                    SetStatusMessage("Project saved.");

                }
                if (ImGui::MenuItem("Save Project as...", "CTRL+SHIFT+S"))
                {
                    std::string savePath = FileDialog::SaveFile("*.project");
                    Engine::GetProject()->SaveAs(savePath);

                    Selection = EditorSelection();
                }
                ImGui::Separator();
                if (ImGui::MenuItem("Set current scene as default"))
                {
                    Engine::GetProject()->DefaultScene = Engine::GetCurrentScene();
                    SetStatusMessage("Current scene set as project default.");
                }
                ImGui::Separator();
                if (ImGui::MenuItem("New scene"))
                {
                    Engine::SetCurrentScene(Scene::New());
                    Selection = EditorSelection();
                    SetStatusMessage("New scene created.");
                }
                if (ImGui::MenuItem("Open scene...", "CTRL+O"))
                {
                    OpenScene();
                    Selection = EditorSelection();
                }
                if (ImGui::MenuItem("Save scene", "CTR+SHIFT+L+S"))
                {
                    Engine::GetCurrentScene()->Save();
                    SetStatusMessage("Scene saved succesfully.");
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
                if (ImGui::MenuItem("Project Settings", "")) 
                {
                    m_ProjectSettingsWindow->m_DisplayWindow = true;
                }
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("View"))
            {
                if (ImGui::MenuItem("Draw grid", NULL, m_DrawGrid))
                {
                    m_DrawGrid = !m_DrawGrid;
                }

                if (ImGui::MenuItem("Draw Axis", NULL, m_DrawAxis))
                {
                    m_DrawAxis = !m_DrawAxis;
                }

                if (ImGui::MenuItem("Draw collisions", NULL, m_DebugCollisions))
                {
                    m_DebugCollisions = !m_DebugCollisions;
                    PhysicsManager::Get().SetDrawDebug(m_DebugCollisions);
                }

                if (ImGui::MenuItem("Draw navigation meshes", NULL, m_DrawNavMesh))
                {

                }

                if (ImGui::MenuItem("Settings", NULL)) {}
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Tool"))
            {
                if (ImGui::MenuItem("Trenchbroom Configurator", NULL, m_ShowTrenchbroomConfigurator))
                {
                    m_ShowTrenchbroomConfigurator = !m_ShowTrenchbroomConfigurator;
                }

                if (ImGui::MenuItem("Map Importer", NULL, m_ShowMapImporter))
                {
                    m_ShowMapImporter = !m_ShowMapImporter;
                }

                if (ImGui::MenuItem("Generate VisualStudio solution", NULL))
                {
                    Nuake::ScriptingEngineNet::Get().GenerateSolution(FileSystem::Root, Engine::GetProject()->Name);
                    Nuake::Logger::Log("Generated Solution.");
                    SetStatusMessage("Visual studio solution generated succesfully.");
                }

                if (ImGui::MenuItem("Generate Trenchbroom config", NULL))
                {
                    PushCommand(CreateTrenchbroomGameConfig(Engine::GetProject()));
                }

#ifdef NK_DEBUG
                if (ImGui::MenuItem("Copy Nuake.NET", NULL))
                {
                    Nuake::ScriptingEngineNet::Get().CopyNuakeNETAssemblies(FileSystem::Root);
                    Nuake::Logger::Log("Copied Nuake.Net Assemblies.");
                    SetStatusMessage("Nuake.Net assemblies succesfully copied.");
                }
#endif // NK_DEBUG

                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Entity"))
            {
                if (ImGui::BeginMenu("Create new"))
                {
                    Entity entity;
                    if (ImGui::MenuItem("Empty"))
                    {
                        entity = Engine::GetCurrentScene()->CreateEntity("Empty entity");
                    }
                    if (ImGui::MenuItem("Light"))
                    {
                        entity = Engine::GetCurrentScene()->CreateEntity("Light");
                        entity.AddComponent<LightComponent>();
                    }
                    if (ImGui::MenuItem("Camera"))
                    {
                        entity = Engine::GetCurrentScene()->CreateEntity("Camera");
                        entity.AddComponent<CameraComponent>();
                    }
                    if (ImGui::MenuItem("Rigidbody"))
                    {
                        entity = Engine::GetCurrentScene()->CreateEntity("Rigidbody");
                        entity.AddComponent<RigidBodyComponent>();
                    }
                    if (ImGui::MenuItem("Trenchbroom map"))
                    {
                        entity = Engine::GetCurrentScene()->CreateEntity("Trenchbroom map");
                        entity.AddComponent<QuakeMapComponent>();
                    }
                    if (ImGui::MenuItem("Model"))
                    {
                        entity = Engine::GetCurrentScene()->CreateEntity("Model");
                        entity.AddComponent<ModelComponent>();
                    }

                    if (entity.IsValid())
                    {
                        if (Selection.Type == EditorSelectionType::Entity && Selection.Entity.IsValid())
                        {
                            Selection.Entity.AddChild(entity);
                        }
                        else
                        {
                            auto camera = Engine::GetCurrentScene()->m_EditorCamera;
                            Vector3 newEntityPos = camera->Translation + camera->Direction;
                            entity.GetComponent<TransformComponent>().SetLocalPosition(newEntityPos);
                        }

                        Selection = EditorSelection(entity);
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
                    SetStatusMessage("Shaders rebuilt.");
                }



                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Quit")) ImGui::EndMenu();
            ImGui::EndMainMenuBar();
        }
    }

    bool isLoadingProject = false;
    bool isLoadingProjectQueue = false;
    bool EditorInterface::isCreatingNewProject = false;
    UIDemoWindow m_DemoWindow;

    int frameCount = 2;
    void EditorInterface::Draw()
    {
		ZoneScoped;

        Init();

        if (isCreatingNewProject && !_NewProjectWindow->HasCreatedProject())
        {
            _NewProjectWindow->Draw();
        }

        if (isLoadingProjectQueue)
        {
            _WelcomeWindow->LoadQueuedProject();
            isLoadingProjectQueue = false;

            auto window = Window::Get();
            window->SetDecorated(true);
            window->SetSize({ 1100, 1000 });
            window->Maximize();
            window->Center();
            frameCount = 0;
            return;
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
        else
        {
            m_ProjectSettingsWindow->Init(Engine::GetProject());
        }

        // Shortcuts
        if(ImGui::IsKeyDown(ImGuiKey_LeftCtrl))
        {
            if(ImGui::IsKeyPressed(ImGuiKey_S))
            {
                Engine::GetProject()->Save();
                Engine::GetCurrentScene()->Save();

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

        pInterface.m_CurrentProject = Engine::GetProject();

        m_ProjectSettingsWindow->Draw();

        m_DemoWindow.Draw();

        _audioWindow->Draw();

        if (m_ShowTrenchbroomConfigurator)
        {
            m_TrenchhbroomConfigurator.Draw();
        }

        if (m_ShowMapImporter)
        {
            m_MapImporter.Draw();
        }

        DrawMenuBar();
        DrawMenuBars();

        int i = 0;
       //if (Logger::GetLogCount() > 0 && Logger::GetLogs()[Logger::GetLogCount() - 1].type == COMPILATION)
       //{
       //    SetStatusMessage(std::string(ICON_FA_EXCLAMATION_TRIANGLE) + "  An unhandled exception occured in your script. See logs for more details.", Color(1.0f, 0.1f, 0.1f, 1.0f));
       //}

        DrawStatusBar();

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
		ZoneScoped;

        if (!Engine::GetCurrentScene() || Engine::IsPlayMode())
        {
            return;
        }

        auto& editorCam = Engine::GetCurrentScene()->m_EditorCamera;
        editorCam->Update(ts, m_IsHoveringViewport && m_IsViewportFocused);

        const bool entityIsSelected = Selection.Type == EditorSelectionType::Entity && Selection.Entity.IsValid();
        if (editorCam->IsFlying() && entityIsSelected && Input::IsKeyPressed(Key::F))
        {
            editorCam->IsMoving = true;
            editorCam->TargetPos = Selection.Entity.GetComponent<TransformComponent>().GetGlobalPosition();
        }

        if (entityIsSelected && Selection.Type == EditorSelectionType::Entity && Selection.Entity.IsValid() && Selection.Entity.HasComponent<CameraComponent>())
        {
            displayVirtualCameraOverlay = true;
        }
        else
        {
            displayVirtualCameraOverlay = false;
        }

        if (entityIsSelected && Input::IsKeyPressed(Key::ESCAPE))
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

        if (entity.HasComponent<AudioEmitterComponent>())
        {
            entityTypeName = "Audio Emitter";
        }

        if (entity.HasComponent<ParticleEmitterComponent>())
        {
            entityTypeName = "Particle Emitter";
        }

        if (entity.HasComponent<QuakeMapComponent>())
        {
            entityTypeName = "Quake Map";
        }

        if (entity.HasComponent<ModelComponent>())
        {
            entityTypeName = "Model";
        }

        if (entity.HasComponent<SkinnedModelComponent>())
        {
            entityTypeName = "Skinned Model";
        }

        return entityTypeName;
    }

    void EditorInterface::PushCommand(ICommand&& command)
    {
        mCommandBuffer->PushCommand(command);
    }

    void EditorInterface::OnWindowFocused()
    {
        filesystem->Scan();
        
    }

    void EditorInterface::OnDragNDrop(const std::vector<std::string>& paths)
    {
        if (Engine::GetProject())
        {
            for (const auto& path : paths)
            {
                if (!FileSystem::FileExists(path, true))
                {
                    continue;
                }

                if (!FileSystem::DirectoryExists(FileSystemUI::m_CurrentDirectory->GetFullPath(), true))
                {
                    return;
                }

                FileSystem::CopyFileAbsolute(path, FileSystemUI::m_CurrentDirectory->GetFullPath());
            }
        }
    }

	bool EditorInterface::LoadProject(const std::string& projectPath)
	{
        FileSystem::SetRootDirectory(FileSystem::GetParentPath(projectPath));

        auto project = Project::New();
        auto projectFileData = FileSystem::ReadFile(projectPath, true);
        Logger::Log("Reading file project: " + projectFileData, "window", VERBOSE);
        try
        {
            Logger::Log("Starting deserializing", "window", VERBOSE);
            project->Deserialize(json::parse(projectFileData));
            project->FullPath = projectPath;

            Engine::LoadProject(project);

            filesystem->m_CurrentDirectory = Nuake::FileSystem::RootDirectory;
        }
        catch (std::exception exception)
        {
            Logger::Log("Error loading project: " + projectPath, "editor", CRITICAL);
            SetStatusMessage("Error loading project: " + projectPath);
            Logger::Log(exception.what());
            return false;
        }

        return true;
	}

}

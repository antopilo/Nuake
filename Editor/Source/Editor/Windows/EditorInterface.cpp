#include <map>
#include <algorithm>

#include "ProjectInterface.h"
#include "EditorInterface.h"

#include "Engine.h"
#include "Nuake/Core/Logger.h"

#include "imgui/imgui.h"
#include "imgui/ImGuizmo.h"
#include "Nuake/Scene/Entities/ImGuiHelper.h"

#include "Nuake/Rendering/Textures/Texture.h"
#include "Nuake/Rendering/Textures/MaterialManager.h"

#include "Nuake/Physics/PhysicsManager.h"

#include "glm/gtc/type_ptr.hpp"
#include "glm/gtx/matrix_decompose.hpp"
#include "Nuake/Resource/FontAwesome5.h"

#include "Nuake/Scene/Scene.h"
#include "Nuake/Scene/Components.h"
#include "Nuake/Scene/Systems/QuakeMapBuilder.h"
#include "../UIComponents/Viewport.h"
#include <Nuake/Resource/Prefab.h>
#include <Nuake/Rendering/Shaders/ShaderManager.h>
#include "Nuake/Rendering/Renderer.h"
#include "Nuake/Core/Input.h"

#include "../Actions/EditorSelection.h"
#include "FileSystemUI.h"

#include <Nuake/FileSystem/Directory.h>

#include "../Misc/InterfaceFonts.h"

#include "WelcomeWindow.h"
#include "LoadingSplash.h"

#include "Nuake/Rendering/SceneRenderer.h"
#include <Nuake/Rendering/Buffers/Framebuffer.h>
#include "UIDemoWindow.h"
#include <Nuake/Audio/AudioManager.h>

#include <Nuake/UI/ImUI.h>
#include "Nuake/FileSystem/FileSystem.h"
#include <Nuake/Resource/StaticResources.h>
#include <Nuake/Threading/JobSystem.h>
#include "../Commands/Commands/Commands.h"
#include <Nuake/Resource/ModelLoader.h>
#include "../ScriptingContext/ScriptingContext.h"
#include <Nuake/Scene/Components/BSPBrushComponent.h>

#include <Nuake/FileSystem/FileDialog.h>

#include "Nuake/Rendering/Vulkan/VulkanRenderer.h"
#include <Nuake/Rendering/Vulkan/VkResources.h>
#include <Nuake/Rendering/Vulkan/VulkanImage/VulkanImage.h>
#include <volk/volk.h>

#include "Nuake/Rendering/Vulkan/SceneRenderPipeline.h"
#include <Nuake/Rendering/Vulkan/DebugCmd.h>

#include "../Events/EditorRequests.h"

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
        
        // Load textures
        NuakeTexture = Nuake::TextureManager::Get()->GetTexture2("Resources/Images/editor-icon.png");
        CloseIconTexture = Nuake::TextureManager::Get()->GetTexture2("Resources/Images/close-icon.png");
        MaximizeTexture = Nuake::TextureManager::Get()->GetTexture2("Resources/Images/maximize-icon.png");
        RestoreTexture = Nuake::TextureManager::Get()->GetTexture2("Resources/Images/restore-icon.png");
        MinimizeTexture = Nuake::TextureManager::Get()->GetTexture2("Resources/Images/minimize-icon.png");
        
        Logger::Log("Creating editor windows", "window", VERBOSE);
        filesystem = new FileSystemUI(this);
        
        //floatingFileBrowser = CreateScope<FileSystemUI>(this);

        _WelcomeWindow = new WelcomeWindow(this);
        _NewProjectWindow = new NewProjectWindow(this);
        _audioWindow = new AudioWindow();
        m_ProjectSettingsWindow = new ProjectSettingsWindow();
        SelectionPanel = new EditorSelectionPanel(this->Selection);
            
        Logger::Log("Building fonts", "window", VERBOSE);
        BuildFonts();
        
        Logger::Log("Loading imgui from mem", "window", VERBOSE);
        using namespace Nuake::StaticResources;
        ImGui::LoadIniSettingsFromMemory((const char*)StaticResources::Data_default_layout_ini);
        
        virtualCamera = CreateRef<FrameBuffer>(true, Vector2{ 640, 360 });
        //ScriptingContext::Get().Initialize();
        
        Window::Get()->SetTitlebarHitTestCallback([&](Window& window, int x, int y, bool& hit) {
            hit = m_TitleBarHovered;
        });

        EditorRequests& requests = EditorRequests::Get();
        requests.OnRequestLoadScene().AddRaw(this, &EditorInterface::OnRequestLoadScene);
        requests.OnRequestCloseEditorWindow().AddRaw(this, &EditorInterface::OnRequestCloseEditorWindow);

		Engine::OnSceneLoaded.AddRaw(this, &EditorInterface::OnSceneLoaded);
    }

    void EditorInterface::DrawTitlebar(float& outHeight)
    {
        const bool isMaximized = Window::Get()->IsMaximized();
        const float titlebarHeight = isMaximized ? 68.0f : 58.0f;
        float titlebarVerticalOffset = isMaximized ? 0.0f : 0.0f;
        const ImVec2 windowPadding = ImGui::GetCurrentWindow()->WindowPadding;

        ImGui::SetCursorPos(ImVec2(windowPadding.x, windowPadding.y + titlebarVerticalOffset));
        const ImVec2 titlebarMin = ImGui::GetCursorScreenPos();
        const ImVec2 titlebarMax = { ImGui::GetCursorScreenPos().x + ImGui::GetWindowWidth() - windowPadding.y * 2.0f,
                                     ImGui::GetCursorScreenPos().y + titlebarHeight };
        auto* bgDrawList = ImGui::GetBackgroundDrawList();
        auto* fgDrawList = ImGui::GetForegroundDrawList();

        // Logo
        {
            const int logoWidth = NuakeTexture->GetSize().x;
            const int logoHeight = NuakeTexture->GetSize().y;
            const ImVec2 logoOffset(2.0f + windowPadding.x, 5.0f + windowPadding.y + titlebarVerticalOffset);
            const ImVec2 logoRectStart = { ImGui::GetItemRectMin().x + logoOffset.x, ImGui::GetItemRectMin().y + logoOffset.y };
            const ImVec2 logoRectMax = { logoRectStart.x + logoWidth, logoRectStart.y + logoHeight };

            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 2.0f + windowPadding.x);
            ImGui::SetCursorPosY(ImGui::GetCursorPosY() + (5.0f + windowPadding.y + titlebarVerticalOffset) / 2.0);
            ImGui::Image((ImTextureID)NuakeTexture->GetImGuiDescriptorSet(), ImVec2(logoWidth, logoHeight));
        }

        const float w = ImGui::GetContentRegionAvail().x;
        const float buttonsAreaWidth = 94;

        // Title bar drag area
        // On Windows we hook into the GLFW win32 window internals
        ImGui::SetCursorPos(ImVec2(windowPadding.x, windowPadding.y + titlebarVerticalOffset)); // Reset cursor pos

        const auto titleBarDragSize = ImVec2(w - buttonsAreaWidth, titlebarHeight);

        if (Window::Get()->IsMaximized())
        {
            float windowMousePosY = ImGui::GetMousePos().y - ImGui::GetCursorScreenPos().y;
            if (windowMousePosY >= 0.0f && windowMousePosY <= 5.0f)
                m_TitleBarHovered = true; // Account for the top-most pixels which don't register
        }

        auto curPos = ImGui::GetCursorPos();
        bool isOnMenu = false;
        {
            const float logoHorizontalOffset = 5.0f * 2.0f + 48.0f + windowPadding.x;
            ImGui::SetCursorPos(ImVec2(logoHorizontalOffset, 6.0f + titlebarVerticalOffset));
            DrawMenuBar();
            if (ImGui::IsItemHovered())
            {
                isOnMenu = true;
            }

        }


        {
            // Centered Window title
            ImVec2 currentCursorPos = ImGui::GetCursorPos();
            std::string title = "Nuake Engine - " + Engine::GetProject()->Name;

            if (Engine::GetProject()->IsDirty)
                title += "*";

            ImVec2 textSize = ImGui::CalcTextSize(title.c_str());
            ImGui::SetCursorPos(ImVec2(ImGui::GetWindowWidth() * 0.5f - textSize.x * 0.5f - ImGui::GetStyle().WindowPadding.x / 2.0f, 2.0f + windowPadding.y + 6.0f));
            ImGui::Text(title.c_str()); // Draw title
            ImGui::SetCursorPos(currentCursorPos);
        }
        ImGui::SetItemAllowOverlap();
        // Window buttons
        const ImU32 buttonColN = UI::TextCol;
        const ImU32 buttonColH = UI::TextCol;
        const ImU32 buttonColP = UI::TextCol;
        const float buttonWidth = 14.0f;
        const float buttonHeight = 14.0f;

        // Minimize Button
        ImGui::SameLine();

        const float remaining = ImGui::GetContentRegionAvail().x;
        ImGui::Dummy(ImVec2(remaining - ((buttonWidth + ImGui::GetStyle().ItemSpacing.x) * 3.5), 0));
        ImGui::SameLine();
        {
            int iconWidth = std::max(MinimizeTexture->GetWidth(), 24);
            int iconHeight = std::max(MinimizeTexture->GetHeight(), 24);

            if (ImGui::InvisibleButton("Minimize", ImVec2(buttonWidth, buttonHeight)))
            {
                //glfwIconifyWindow(Window::Get()->GetHandle());
            }

            auto rect = ImRect(ImGui::GetItemRectMin(), ImGui::GetItemRectMax());
            UI::DrawButtonImage(MinimizeTexture, buttonColN, buttonColH, buttonColP, rect);
        }

        ImGui::SameLine();

        // Maximize Button
        {
            int iconWidth = std::max(MaximizeTexture->GetWidth(), 24);
            int iconHeight = std::max(MaximizeTexture->GetHeight(), 24);

            const bool isMaximized = Window::Get()->IsMaximized();

            if (ImGui::InvisibleButton("Maximize", ImVec2(buttonWidth, buttonHeight)))
            {
                const auto window = Window::Get()->GetHandle();
                if (isMaximized)
                {
                    //glfwRestoreWindow(window);
                }
                else
                {
                    //glfwMaximizeWindow(window);
                }
            }
            auto rect = ImRect(ImGui::GetItemRectMin(), ImGui::GetItemRectMax());
            UI::DrawButtonImage(isMaximized ? RestoreTexture : MaximizeTexture, buttonColN, buttonColH, buttonColP);
        }

        // Close Button
        ImGui::SameLine();
        {
            int iconWidth = std::max(CloseIconTexture->GetWidth(), 24);
            int iconHeight = std::max(CloseIconTexture->GetHeight(), 24);
            if (ImGui::InvisibleButton("Close", ImVec2(buttonWidth, buttonHeight)))
            {
                //glfwSetWindowShouldClose(Window::Get()->GetHandle(), true);
            }

            UI::DrawButtonImage(CloseIconTexture, UI::TextCol, UI::TextCol, buttonColP);
        }

        // Second bar with play stop pause etc
        ImGui::SetCursorPosX(windowPadding.x + NuakeTexture->GetSize().x);
        ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 2.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(2, 2));
        ImGui::PushStyleColor(ImGuiCol_Button, IM_COL32(0, 0, 0, 0));

        //ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(20, ImGui::GetStyle().WindowPadding.y));
        //ImGui::PushStyleVar(ImGuiStyleVar_TabBarBorderSize, 0);
        //
        //ImGui::SetNextWindowSize({ 300.0f, 25.0f });
        //if (ImGui::BeginTabBar("SceneTabsBar", ImGuiTabBarFlags_None))
        //{
        //    ImGui::BeginTabItem("Scene 1");
        //    ImGui::BeginTabItem("Scene 2");
        //    ImGui::BeginTabItem("Scene 3");
        //    ImGui::EndTabItem();
        //
        //    ImGui::EndTabBar();
        //}
        //
        //ImGui::PopStyleVar(2);
        //
        //
        //ImGui::SameLine();
        ImGui::Dummy({ ImGui::GetContentRegionAvail().x / 2.0f - (76.0f / 2.0f), 8.0f });
        ImGui::SameLine();
        //ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 6.0f);


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

            if (ImGui::IsItemHovered())
            {
                isOnMenu = true;
            }

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

            if (ImGui::IsItemHovered())
            {
                isOnMenu = true;
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
                            auto project = Engine::GetProject();
                            auto& scriptingEngine = ScriptingEngineNet::Get();
                            const std::string& assemblyPath = scriptingEngine.GetGameAssemblyPath(project);
                            if (FileSystem::FileExists(assemblyPath))
                            {
                                this->errors = ScriptingEngineNet::Get().BuildProjectAssembly(Engine::GetProject());
                                FileSystem::GetFile(assemblyPath)->SetHasBeenModified(false);
                            }
                    };

                    Selection = EditorSelection();

                    JobSystem::Get().Dispatch(job, [this]()
                    {
                        bool containsError = false;
                        std::find_if(errors.begin(), errors.end(), [](const CompilationError& error) {
                            return error.isWarning == false;
                        });

                        if (errors.size() > 0 && containsError)
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

                            ImGui::SetWindowFocus("Logger");
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

        if (ImGui::IsItemHovered())
        {
            isOnMenu = true;
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

        ImVec2 curPos2 = ImGui::GetCursorPos();
        ImVec2 min = ImVec2(curPos2.x - separatorThickness, curPos2.y - separatorHeight);
        ImVec2 max = ImVec2(curPos2.x + separatorThickness, curPos2.y - separatorHeight);
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

        static bool isBuilding = false;
        std::string icon = isBuilding ? ICON_FA_SYNC_ALT : ICON_FA_HAMMER;

        if (isBuilding)
        {
            ImGui::BeginDisabled();
        }

        if (ImGui::Button(icon.c_str(), ImVec2(30, 30)) && !isBuilding)
        {
            SetStatusMessage(std::string(ICON_FA_HAMMER) + " Building solution...", { 0.1f, 0.1f, 1.0f, 1.0f });

            auto job = [this]()
                {
                    isBuilding = true;
                    this->errors = ScriptingEngineNet::Get().BuildProjectAssembly(Engine::GetProject());
                };

            JobSystem::Get().Dispatch(job, [this]()
            {
            isBuilding = false;
            bool containsError = false;
            std::find_if(errors.begin(), errors.end(), [](const CompilationError& error) {
                return error.isWarning == false;
                });

            if (errors.size() > 0 && containsError)
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
                for (CompilationError error : errors)
                {
                    const std::string errorMessage = error.file + "( line " + std::to_string(error.line) + "): " + error.message;
                    Logger::Log(errorMessage, ".net", WARNING);
                }

                Logger::Log("Build Successful!", ".net", VERBOSE);
                ScriptingEngineNet::Get().LoadProjectAssembly(Engine::GetProject());
                Engine::GetProject()->ExportEntitiesToTrenchbroom();
                SetStatusMessage("Build Successful!");
            }
            });
        }

        if (isBuilding)
        {
            ImGui::EndDisabled();
        }

        if (ImGui::BeginItemTooltip())
        {
            ImGui::Text("Built .Net project");
            ImGui::EndTooltip();
        }

        ImGui::SetCursorPos(curPos);
        ImGui::InvisibleButton("##titleBarDragZone", ImVec2(w - buttonsAreaWidth, titlebarHeight));
        m_TitleBarHovered = ImGui::IsItemHovered();

        if (isOnMenu)
        {
            m_TitleBarHovered = false;
        }

        ImGui::SetItemAllowOverlap();

        ImGui::PopStyleColor();
        ImGui::PopStyleVar();

        ImGui::PopStyleColor();
        ImGui::PopStyleVar();

        ImGui::PopStyleVar(1);

        outHeight = titlebarHeight;
    }

    void EditorInterface::Init()
    {
        ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
        static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_AutoHideTabBar;

        //ImGuiViewport* viewport = ImGui::GetMainViewport();
        //ImGui::SetNextWindowPos(viewport->Pos);
        //ImGui::SetNextWindowSize(viewport->Size);
        //ImGui::SetNextWindowViewport(viewport->ID);
        //
        //ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
        //ImGui::DockSpaceOverViewport(viewport, dockspace_flags);
    }

    ImVec2 LastSize = ImVec2();
    void EditorInterface::DrawViewport()
    {
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
        std::string name = ICON_FA_GAMEPAD + std::string("  Scene");
        ImGuiWindowClass window_class;
        //window_class.DockNodeFlagsOverrideSet = ImGuiDockNodeFlags_NoTabBar;
        //ImGui::SetNextWindowClass(&window_class);

        if (ImGui::Begin(name.c_str(), 0, ImGuiWindowFlags_NoDecoration))
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
            VkDescriptorSet textureDesc = VkRenderer::Get().DrawImage->GetImGuiDescriptorSet();
            if (SelectedViewport == 1)
            {
				//pipeline.GetRenderPass("Shadow").GetDepthAttachment().Image->TransitionLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
                //textureDesc = pipeline.GetRenderPass("Shadow").GetDepthAttachment().Image->GetImGuiDescriptorSet();
                texture = Engine::GetCurrentScene()->m_SceneRenderer->GetGBuffer().GetTexture(0);
            }
            else if (SelectedViewport == 2)
            {
                //textureDesc = pipeline.GetRenderPass("GBuffer").GetAttachment("Normal").Image->GetImGuiDescriptorSet();
                //texture = Engine::GetCurrentScene()->m_SceneRenderer->GetGBuffer().GetTexture(GL_COLOR_ATTACHMENT1);
            }
            else if (SelectedViewport == 3)
            {
                //textureDesc = pipeline.GetRenderPass("GBuffer").GetDepthAttachment().Image->GetImGuiDescriptorSet();
                //texture = Engine::GetCurrentScene()->m_SceneRenderer->GetScaledDepthTexture();
            }
            else if (SelectedViewport == 4)
            {
                //textureDesc = pipeline.GetRenderPass("GBuffer").GetAttachment("Material").Image->GetImGuiDescriptorSet();
                //texture = Engine::GetCurrentScene()->m_SceneRenderer->GetVelocityTexture();
            }

            ImVec2 imagePos = ImGui::GetWindowPos() + ImGui::GetCursorPos();
            Input::SetEditorViewportSize(m_ViewportPos, viewportPanelSize);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
            m_ViewportPos = { imagePos.x, imagePos.y };
            ImGui::Image(textureDesc, regionAvail, {0, 1},  {1, 0});
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
                Vector2 textureSize = gbuffer.GetTexture(0)->GetSize();
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
                    //char* file = (char*)payload->Data;
                    //std::string fullPath = std::string(file, 256);
                    //fullPath = Nuake::FileSystem::AbsoluteToRelative(fullPath);
                    //
                    //auto loader = ModelLoader();
                    //auto modelResource = loader.LoadModel(fullPath);
                    //
                    //auto entity = Engine::GetCurrentScene()->CreateEntity(FileSystem::GetFileNameFromPath(fullPath));
                    //ModelComponent& modelComponent = entity.AddComponent<ModelComponent>();
                    //modelComponent.ModelPath = fullPath;
                    //modelComponent.ModelResource = modelResource;
                    //entity.GetComponent<TransformComponent>().SetLocalPosition(dragnDropWorldPos);
                }

                if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("_Map"))
                {
                    char* file = (char*)payload->Data;
                    std::string fullPath = std::string(file, 256);
                    fullPath = Nuake::FileSystem::AbsoluteToRelative(fullPath);

                    auto entity = Engine::GetCurrentScene()->CreateEntity(FileSystem::GetFileNameFromPath(fullPath));
                    QuakeMapComponent& mapComponent = entity.AddComponent<QuakeMapComponent>();
                    mapComponent.Path = FileSystem::GetFile(fullPath);
                    mapComponent.AutoRebuild = true;
                    mapComponent.HasCollisions = true;
                    mapComponent.ActionRebuild();
                }
            }
            else
            {
                Engine::GetCurrentScene()->m_SceneRenderer->ClearTemporaryModels();
            }

            ImGuizmo::SetDrawlist();
            ImGuizmo::AllowAxisFlip(true);
            ImGuizmo::SetRect(imagePos.x, imagePos.y, viewportPanelSize.x, viewportPanelSize.y);
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

                    // Since imguizmo doesnt support reverse-Z, we need to create a new projection matrix
                    // With a normal near and far plane.
                    Matrix4 normalZProjection = glm::perspectiveFov(glm::radians(editorCam->Fov), 9.0f * editorCam->AspectRatio, 9.0f, editorCam->Far, editorCam->Near);

                    static Vector3 camPreviousPos = Engine::GetCurrentScene()->m_EditorCamera->Translation;
                    static Vector3 camNewPos = Vector3(0, 0, 0);
                    Vector3 camDelta = camNewPos - camPreviousPos;
                    Vector3 previousGlobalPos = transform[3];
                    // Imguizmo calculates the delta from the gizmo,
                    ImGuizmo::Manipulate(
                        glm::value_ptr(Engine::GetCurrentScene()->GetCurrentCamera()->GetTransform()),
                        glm::value_ptr(normalZProjection),
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

                        Vector3 newGlobalPos = transform[3];
                        if(ImGui::IsKeyDown(ImGuiKey_LeftShift))
                        {
                            Vector3 positionDelta = newGlobalPos - previousGlobalPos;
                            Engine::GetCurrentScene()->m_EditorCamera->Translation += positionDelta;
                            camNewPos = Engine::GetCurrentScene()->m_EditorCamera->Translation;
                        }

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

            if (ImGui::IsWindowHovered() && m_IsHoveringViewport && !m_IsViewportFocused)
            {
                ImGui::FocusWindow(ImGui::GetCurrentWindow());
            }

            m_IsViewportFocused = ImGui::IsWindowFocused();

            if (!Engine::IsPlayMode() && ImGui::GetIO().WantCaptureMouse && m_IsHoveringViewport && !ImGuizmo::IsUsing() && m_IsViewportFocused)
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
                if (ImGui::Button("File Browser"))
                {
                    this->showFloatingFileBrowser = !this->showFloatingFileBrowser;
                }

                if (ImGui::Button("Logger"))
                {
                    this->showFloatingLogger = !this->showFloatingLogger;
                }

                ImGui::Text(m_StatusMessage.c_str());

                ImGui::SameLine();

                const float remainingWidth = ImGui::GetContentRegionAvail().x;
                auto nuakeLogoTexture = TextureManager::Get()->GetTexture2("Resources/Images/logo_white.png");
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
                ImGui::Image((ImTextureID)(nuakeLogoTexture->GetImGuiDescriptorSet()), ImVec2(logoWidth, height) * scale);
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

            
        }
        ImGui::PopStyleVar();
    }

    static int selected = 0;
    Entity QueueDeletion;
    void EditorInterface::DrawEntityTree(Entity e, bool drawChildrens)
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
        bool isPrefab = e.HasComponent<PrefabComponent>();
        if (parent.Children.size() <= 0 || isPrefab || !drawChildrens)
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
        ImVec2 cursorPosition = ImGui::GetCursorScreenPos();
        const auto& cleanName = String::RemoveWhiteSpace(String::ToUpper(name));
        const size_t searchIt = cleanName.find(String::RemoveWhiteSpace(String::ToUpper(searchQuery)));

        ImGui::SetNextItemAllowOverlap();
        bool open = ImGui::TreeNodeEx(name.c_str(), base_flags);

        if (!searchQuery.empty() && searchIt != std::string::npos)
        {
            int firstLetterFoundIndex = static_cast<int>(searchIt);

            const auto foundStr = name.substr(0, firstLetterFoundIndex + searchQuery.size());
            auto highlightBeginPos = ImGui::CalcTextSize(foundStr.c_str());
            auto highlightEndPos = ImGui::CalcTextSize(searchQuery.c_str());

            auto fg = ImGui::GetForegroundDrawList();
            auto color = Engine::GetProject()->Settings.PrimaryColor;
            auto rgbColor = IM_COL32(color.r * 255.0f, color.g * 255.0f, color.b * 255.0f, std::min(color.a, 0.2f) * 255.0f);

            fg->AddRectFilled(ImVec2(cursorPosition.x + 20.0f, cursorPosition.y + 4.0f), ImVec2(cursorPosition.x + highlightEndPos.x + 26.0f, cursorPosition.y + highlightEndPos.y + 6.0f), rgbColor, 4.0f);
        }

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

		if (!m_IsRenaming && ImGui::BeginDragDropSource())
		{ 
			ImGui::SetDragDropPayload("ENTITY", (void*)&e, sizeof(Entity));
			ImGui::Text(name.c_str());
			ImGui::EndDragDropSource();
		}

        if (!isPrefab && ImGui::BeginDragDropTarget())
        {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ENTITY"))
            {
                Entity payload_entity = *(const Entity*)payload->Data;

                // Check if entity is already parent.
                ParentComponent& parentPayload = payload_entity.GetComponent<ParentComponent>();
                if (!payload_entity.EntityContainsItself(payload_entity, e) && parentPayload.Parent != e && std::count(parent.Children.begin(), parent.Children.end(), payload_entity) == 0)
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
            else if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("_NetScript"))
            {
                char* file = (char*)payload->Data;

                std::string fullPath = std::string(file, 512);
                std::string path = Nuake::FileSystem::AbsoluteToRelative(std::move(fullPath));

                if (e.HasComponent<NetScriptComponent>())
                {
                    e.GetComponent<NetScriptComponent>().ScriptPath = path;
                }
                else
                {
                    e.AddComponent<NetScriptComponent>().ScriptPath = path;
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

            if (!isPrefab && ImGui::Selectable("Save entity as a new prefab"))
            {
                Ref<Prefab> newPrefab = Prefab::CreatePrefabFromEntity(Selection.Entity);
                std::string savePath = FileDialog::SaveFile("*.prefab");
                if (!String::EndsWith(savePath, ".prefab"))
                {
                    savePath += ".prefab";
                }

                if (!savePath.empty()) 
                {
                    newPrefab->SaveAs(FileSystem::AbsoluteToRelative(savePath));
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
            if ((drawChildrens && !isPrefab))
            {
                // Caching list to prevent deletion while iterating.
                std::vector<Entity> childrens = parent.Children;
                for (auto& c : childrens)
                    DrawEntityTree(c);
            }

            ImGui::TreePop();
        }
        
        ImGui::PopStyleVar();
        ImGui::PopFont();
    }

    

    void EditorInterface::DrawSceneTree()
    {
        Ref<Scene> scene = Engine::GetCurrentScene();
          
        if (!scene)
            return;

        std::string title = ICON_FA_TREE + std::string("   Hierarchy");
        if (ImGui::Begin(title.c_str()))
        {
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
                    //mGui::TableHeadersRow();
                    ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(0, 0));

                    // Build list of valid entity to display
                    std::vector<Entity> entitiesToDisplay;
                    if (searchQuery.empty())
                    {
                        entitiesToDisplay = scene->GetAllEntities();
                    }
                    else
                    {
                        auto view = scene->m_Registry.view<NameComponent>();
                        for (auto& e : view)
                        {
                            auto& nameComponent = view.get<NameComponent>(e);
                            if (String::RemoveWhiteSpace(String::ToUpper(nameComponent.Name)).find(String::RemoveWhiteSpace(String::ToUpper(searchQuery))) != std::string::npos)
                            {
                                entitiesToDisplay.push_back({ e, scene.get() });
                            }
                        }
                    }

                    // Display valid entities
                    for (Entity e : entitiesToDisplay)
                    {
                        // Draw all entity without parents.
                        bool displayAllHierarchy = searchQuery.empty();
                        if ((displayAllHierarchy && !e.GetComponent<ParentComponent>().HasParent) || !displayAllHierarchy)
                        {
                            ImGui::PushFont(normalFont);

                            // Recursively draw childrens if not searching
                            const std::string entityName = e.GetComponent<NameComponent>().Name;

                            DrawEntityTree(e, displayAllHierarchy);

                            ImGui::PopFont();
                        }

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
                    auto newPrefabInstance = Prefab::New(relPath);
                    newPrefabInstance->Root.GetComponent<PrefabComponent>().SetPrefab(newPrefabInstance);
                    newPrefabInstance->Root.GetComponent<NameComponent>().IsPrefab = true;
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

    bool LogErrors = true;
    bool LogWarnings = true;
    bool LogDebug = true;
    bool AutoScroll = true;
    void EditorInterface::DrawLogger()
    {
        if (ImGui::Begin("Logger"))
        {
            if (ImGui::Button("Clear", ImVec2(60, 28)))
            {
                Logger::ClearLogs();
                SetStatusMessage("Logs cleared.");
            }

            ImGui::SameLine();

            if (ImGui::Button(ICON_FA_FILTER, ImVec2(30, 28)))
            {
                ImGui::OpenPopup("filter_popup");
            }

            ImGui::SameLine();

            bool isEnabled = LogErrors;
            if (ImGui::BeginPopup("filter_popup"))
            {
                ImGui::SeparatorText("Filters");
                ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(2, 2));
                ImGui::PushStyleColor(ImGuiCol_Button, IM_COL32(0, 0, 0, 0));
                ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 100);

                if (isEnabled)
                {
                    Color color = Engine::GetProject()->Settings.PrimaryColor;
                    ImGui::PushStyleColor(ImGuiCol_Button, { color.r, color.g, color.b, 1.0f });
                }

                if (ImGui::Button((std::string(ICON_FA_BAN) + " Error").c_str()))
                {
                    LogErrors = !LogErrors;
                }

                UI::Tooltip("Display Errors");
                if (isEnabled)
                {
                    ImGui::PopStyleColor();
                }

                isEnabled = LogWarnings;
                if (isEnabled)
                {
                    Color color = Engine::GetProject()->Settings.PrimaryColor;
                    ImGui::PushStyleColor(ImGuiCol_Button, { color.r, color.g, color.b, 1.0f });
                }

                if (ImGui::Button((std::string(ICON_FA_EXCLAMATION_TRIANGLE) + " Warning").c_str()))
                {
                    LogWarnings = !LogWarnings;
                }

                UI::Tooltip("Display Warnings");
                if (isEnabled)
                {
                    ImGui::PopStyleColor();
                }

                isEnabled = LogDebug;
                if (isEnabled)
                {
                    Color color = Engine::GetProject()->Settings.PrimaryColor;
                    ImGui::PushStyleColor(ImGuiCol_Button, { color.r, color.g, color.b, 1.0f });
                }

                if (ImGui::Button((std::string(ICON_FA_INFO) + " Info").c_str()))
                {
                    LogDebug = !LogDebug;
                }

                UI::Tooltip("Display Verbose");
                if (isEnabled)
                {
                    ImGui::PopStyleColor();
                }

                ImGui::PopStyleColor();
                ImGui::PopStyleVar(2);

                ImGui::EndPopup();
            }

            ImGui::SameLine();

            isEnabled = AutoScroll;
            if (isEnabled)
            {
                Color color = Engine::GetProject()->Settings.PrimaryColor;
                ImGui::PushStyleColor(ImGuiCol_Button, { color.r, color.g, color.b, 1.0f });
            }

            if (ImGui::Button(ICON_FA_ARROW_DOWN, ImVec2(30, 28)))
            {
                AutoScroll = !AutoScroll;
            }

            UI::Tooltip("Auto-Scroll");
            if (isEnabled)
            {
                ImGui::PopStyleColor();
            }

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
        if (Engine::GetGameState() == GameState::Playing)
        {
            return;
        }
         
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

            if (ImGui::Button(ICON_FA_ARROWS_ALT, ImVec2(30, 28)) || (ImGui::Shortcut(ImGuiKey_W, 0, ImGuiInputFlags_RouteGlobalLow) && !ImGui::IsAnyItemActive() && !isControllingCamera))
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

            if (ImGui::Button(ICON_FA_SYNC_ALT, ImVec2(30, 28)) || (ImGui::Shortcut(ImGuiKey_E, 0, ImGuiInputFlags_RouteGlobalLow) && !ImGui::IsAnyItemActive() && !isControllingCamera))
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

            if (ImGui::Button(ICON_FA_EXPAND_ALT, ImVec2(30, 28)) || (ImGui::Shortcut(ImGuiKey_R, 0, ImGuiInputFlags_RouteGlobalLow) && !ImGui::IsAnyItemActive() && !isControllingCamera))
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
            CurrentSnapping = { CurrentSnapping.x, CurrentSnapping.x, CurrentSnapping.x };
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
            const char* items[] = { "Shaded", "Albedo", "Normal", "Depth", "Material", "UV", "SSAO"};
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
        if (isControllingCamera)
        {
            if (ImGui::Begin("Controls", &m_ShowOverlay, window_flags))
            {
                const auto& editorCam = Engine::GetCurrentScene()->m_EditorCamera;
                const float camSpeed = editorCam->Speed;

                const float maxSpeed = 50.0f;
                const float minSpeed = 0.05f;
                const float normalizedSpeed = glm::clamp((camSpeed / maxSpeed), 0.0f, 1.0f);

                ImVec2 start = ImGui::GetWindowPos() - ImVec2(0.0, 4.0);
                ImVec2 end = start + ImGui::GetWindowSize() - ImVec2(0, 16.0);
                ImVec2 startOffset = ImVec2(start.x, end.y - (normalizedSpeed * (ImGui::GetWindowHeight() - 20.0)));

                ImGui::GetWindowDrawList()->AddRectFilled(startOffset + ImVec2(0, 10.0), end + ImVec2(0.0, 20.0), IM_COL32(255, 255, 255, 180), 8.0f, ImDrawFlags_RoundCornersAll);
                ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 100);
                ImGui::PopStyleVar();
            }
            ImGui::End();
        }
        ImGui::PopStyleVar();


        corner = 2;

        if (Selection.Type == EditorSelectionType::Entity && Selection.Entity.IsValid() && Selection.Entity.HasComponent<CameraComponent>() && !Engine::IsPlayMode() && m_DrawCamPreview)
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

                static SceneRenderer sceneRenderer = Nuake::SceneRenderer();
                sceneRenderer.BeginRenderScene(cam->GetPerspective(), cam->GetTransform(), cam->Translation);
                sceneRenderer.RenderScene(*Engine::GetCurrentScene().get(), *virtualCamera.get(), false);

                virtualCamera->Clear();
                //ImGui::Image((void*)virtualCamera->GetTexture()->GetID(), { 640, 360 }, { 0, 1 }, {1, 0});

                ImGui::PopStyleVar(2);
                ImGui::PopStyleColor(4);
            }

            ImGui::PopStyleVar();
            ImGui::End();
        }
    }

    void EditorInterface::OnRequestLoadScene(Ref<File> file)
    {
        OpenSceneWindow(file->GetRelativePath());
    }

    void EditorInterface::OnRequestCloseEditorWindow(std::string windowName)
    {
        for (size_t i = 0; i < std::size(sceneEditors); i++)
        {
            // Erase window from sceneEditors
            if (auto window = sceneEditors[i]; window->GetWindowName() == windowName)
            {
                sceneEditors.erase(std::begin(sceneEditors) + i);
                return;
            }
        }
    }

    void EditorInterface::OpenPrefabWindow(const std::string& prefabPath)
    {
        if (!FileSystem::FileExists(prefabPath))
        {
            return;
        }

        Ref<Prefab> newPrefab = CreateRef<Prefab>();
        newPrefab->Path = prefabPath;

        prefabEditors.push_back(CreateRef<PrefabEditorWindow>(newPrefab));
    }

	void EditorInterface::OpenSceneWindow(const std::string& scenePath)
	{
		if (!FileSystem::FileExists(scenePath))
		{
			return;
		}

		Ref<Scene> newScene = Scene::New();
		newScene->Path = scenePath;
        newScene->Deserialize(json::parse(FileSystem::ReadFile(scenePath)));

        Ref<SceneEditorWindow> sceneEditor = CreateRef<SceneEditorWindow>(newScene);
        sceneEditor->DockTo(SceneEditorDockspaceNodeID);
		sceneEditors.push_back(sceneEditor);
	}

    void NewProject()
    {
        if (Engine::GetProject() && Engine::GetProject()->FileExist())
            Engine::GetProject()->Save();
        
        std::string selectedProject = FileDialog::SaveFile("Project file\0*.project");
        
        if (selectedProject.empty()) // Hit cancel
            return;

        if(!String::EndsWith(selectedProject, ".project"))
            selectedProject += ".project";
        
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
        const ImRect menuBarRect = { ImGui::GetCursorPos(), { ImGui::GetContentRegionAvail().x + ImGui::GetCursorScreenPos().x, ImGui::GetFrameHeightWithSpacing() } };
        ImGui::BeginGroup();
       
        if (BeginMenubar(menuBarRect))
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
                if (ImGui::MenuItem("Draw Grid", NULL, m_DrawGrid))
                {
                    m_DrawGrid = !m_DrawGrid;
                }

                if (ImGui::MenuItem("Draw Axis", NULL, m_DrawAxis))
                {
                    m_DrawAxis = !m_DrawAxis;
                }

                if (ImGui::MenuItem("Draw Shapes", NULL, m_DrawShapes))
                {
                    m_DrawShapes = !m_DrawShapes;
                }

                if (ImGui::MenuItem("Draw Gizmos", NULL, m_DrawGizmos))
                {
                    m_DrawGizmos = !m_DrawGizmos;
                }

                ImGui::Separator();

                if (ImGui::MenuItem("Draw Camera Preview", NULL, m_DrawCamPreview))
                {
                    m_DrawCamPreview = !m_DrawCamPreview;
                }

#ifdef NK_DEBUG
                if (ImGui::MenuItem("Show ImGui", NULL, m_ShowImGuiDemo)) m_ShowImGuiDemo = !m_ShowImGuiDemo;
#endif // NK_DEBUG

               
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

                if (ImGui::MenuItem("GPU Resources", 0, m_ShowGpuResources))
                {
                    m_ShowGpuResources = !m_ShowGpuResources;

                }
#endif // NK_DEBUG

                ImGui::EndMenu();
            }
        }

        EndMenubar();

        if (ImGui::IsItemHovered())
            m_TitleBarHovered = false;

        ImGui::EndGroup();
    }

    bool EditorInterface::BeginMenubar(const ImRect& barRect)
    {
        ImGuiWindow* window = ImGui::GetCurrentWindow();
        if (window->SkipItems)
            return false;
        /*if (!(window->Flags & ImGuiWindowFlags_MenuBar))
            return false;*/

        IM_ASSERT(!window->DC.MenuBarAppending);
        ImGui::BeginGroup(); // Backup position on layer 0 // FIXME: Misleading to use a group for that backup/restore
        ImGui::PushID("##menubar2");

        const ImVec2 padding = window->WindowPadding;
        ImRect result = barRect;
        result.Min.x += 0.0f;
        result.Min.y += padding.y;
        result.Max.x += 0.0f;
        result.Max.y += padding.y;

        // We don't clip with current window clipping rectangle as it is already set to the area below. However we clip with window full rect.
        // We remove 1 worth of rounding to Max.x to that text in long menus and small windows don't tend to display over the lower-right rounded area, which looks particularly glitchy.
        ImRect bar_rect = result;// window->MenuBarRect();
        ImRect clip_rect(IM_ROUND(ImMax(window->Pos.x, bar_rect.Min.x + window->WindowBorderSize + window->Pos.x - 10.0f)), IM_ROUND(bar_rect.Min.y + window->WindowBorderSize + window->Pos.y),
            IM_ROUND(ImMax(bar_rect.Min.x + window->Pos.x, bar_rect.Max.x - ImMax(window->WindowRounding, window->WindowBorderSize))), IM_ROUND(bar_rect.Max.y + window->Pos.y));

        clip_rect.ClipWith(window->OuterRectClipped);
        ImGui::PushClipRect(clip_rect.Min, clip_rect.Max, false);

        // We overwrite CursorMaxPos because BeginGroup sets it to CursorPos (essentially the .EmitItem hack in EndMenuBar() would need something analogous here, maybe a BeginGroupEx() with flags).
        window->DC.CursorPos = window->DC.CursorMaxPos = ImVec2(bar_rect.Min.x + window->Pos.x, bar_rect.Min.y + window->Pos.y);
        window->DC.LayoutType = ImGuiLayoutType_Horizontal;
        window->DC.NavLayerCurrent = ImGuiNavLayer_Menu;
        window->DC.MenuBarAppending = true;
        ImGui::AlignTextToFramePadding();
        return true;
    }

    void EditorInterface::EndMenubar()
    {
        ImGuiWindow* window = ImGui::GetCurrentWindow();
        if (window->SkipItems)
            return;
        ImGuiContext& g = *GImGui;

        // Nav: When a move request within one of our child menu failed, capture the request to navigate among our siblings.
        if (ImGui::NavMoveRequestButNoResultYet() && (g.NavMoveDir == ImGuiDir_Left || g.NavMoveDir == ImGuiDir_Right) && (g.NavWindow->Flags & ImGuiWindowFlags_ChildMenu))
        {
            // Try to find out if the request is for one of our child menu
            ImGuiWindow* nav_earliest_child = g.NavWindow;
            while (nav_earliest_child->ParentWindow && (nav_earliest_child->ParentWindow->Flags & ImGuiWindowFlags_ChildMenu))
                nav_earliest_child = nav_earliest_child->ParentWindow;
            if (nav_earliest_child->ParentWindow == window && nav_earliest_child->DC.ParentLayoutType == ImGuiLayoutType_Horizontal && (g.NavMoveFlags & ImGuiNavMoveFlags_Forwarded) == 0)
            {
                // To do so we claim focus back, restore NavId and then process the movement request for yet another frame.
                // This involve a one-frame delay which isn't very problematic in this situation. We could remove it by scoring in advance for multiple window (probably not worth bothering)
                const ImGuiNavLayer layer = ImGuiNavLayer_Menu;
                IM_ASSERT(window->DC.NavLayersActiveMaskNext & (1 << layer)); // Sanity check
                ImGui::FocusWindow(window);
                ImGui::SetNavID(window->NavLastIds[layer], layer, 0, window->NavRectRel[layer]);
                g.NavDisableHighlight = true; // Hide highlight for the current frame so we don't see the intermediary selection.
                g.NavDisableMouseHover = g.NavMousePosDirty = true;
                ImGui::NavMoveRequestForward(g.NavMoveDir, g.NavMoveClipDir, g.NavMoveFlags, g.NavMoveScrollFlags); // Repeat
            }
        }

        IM_MSVC_WARNING_SUPPRESS(6011); // Static Analysis false positive "warning C6011: Dereferencing NULL pointer 'window'"
        // IM_ASSERT(window->Flags & ImGuiWindowFlags_MenuBar); // NOTE(Yan): Needs to be commented out because Jay
        IM_ASSERT(window->DC.MenuBarAppending);
        ImGui::PopClipRect();
        ImGui::PopID();
        window->DC.MenuBarOffset.x = window->DC.CursorPos.x - window->Pos.x; // Save horizontal position so next append can reuse it. This is kinda equivalent to a per-layer CursorPos.
        g.GroupStack.back().EmitItem = false;
        ImGui::EndGroup(); // Restore position on layer 0
        window->DC.LayoutType = ImGuiLayoutType_Vertical;
        window->DC.NavLayerCurrent = ImGuiNavLayer_Main;
        window->DC.MenuBarAppending = false;
    }

    void EditorInterface::OnSceneLoaded(Ref<Scene> scene)
    {
        Window::Get()->SetDecorated(true);
        VkRenderer::Get().SceneRenderer->sceneRenderPipeline->OnDebugDraw().AddRaw(this, &EditorInterface::OnDebugDraw);
    }

    void EditorInterface::OnDebugDraw(DebugCmd& cmd)
    {
        
		//cmd.DrawLine({ 0, 0, 0 }, { 10, 10, 10 }, { 1, 0, 0, 1 });
		//cmd.DrawCube({ 0, 0, 0 }, { 1, 1, 1 }, { 0, 1, 0, 1 });
    }

    bool isLoadingProject = false;
    bool isLoadingProjectQueue = false;
    bool EditorInterface::isCreatingNewProject = false;
    UIDemoWindow m_DemoWindow;

    int frameCount = 2;
    void EditorInterface::Draw()
    {
        Init();

        if (isCreatingNewProject && !_NewProjectWindow->HasCreatedProject())
        {
            _NewProjectWindow->Draw();
        }

        if (isLoadingProjectQueue)
        {
            _WelcomeWindow->LoadQueuedProject();

            auto project = Engine::GetProject();
            OpenSceneWindow(project->DefaultScene->Path);

            isLoadingProjectQueue = false;

            auto window = Window::Get();
            window->ShowTitleBar(false);
            window->SetSize({ 1600, 900 });
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
            if(ImGui::IsKeyPressed(ImGuiKey_S, false))
            {
                Engine::GetProject()->Save();
                Engine::GetCurrentScene()->Save();

            }
            else if(ImGui::IsKeyPressed(ImGuiKey_O, false))
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

#ifdef NK_DEBUG
        // Shader reloading
        if (ImGui::IsKeyPressed(ImGuiKey_F1, false))
        {
            ShaderManager::RebuildShaders();
        }
#endif

        pInterface.m_CurrentProject = Engine::GetProject();

        uint32_t selectedEntityID;
        if (Selection.Type == EditorSelectionType::Entity && Selection.Entity.IsValid())
        {
           selectedEntityID = Selection.Entity.GetHandle();
        }
        else
        {
            selectedEntityID = 0;
        }

        Nuake::Engine::GetCurrentScene()->m_SceneRenderer->mOutlineEntityID = selectedEntityID;

        m_ProjectSettingsWindow->Draw();

        //m_DemoWindow.Draw();

        //_audioWindow->Draw();

        if (m_ShowTrenchbroomConfigurator)
        {
            m_TrenchhbroomConfigurator.Draw();
        }

        if (m_ShowGpuResources)
        {
            if (ImGui::Begin("GPU Resources"))
            {
                GPUResources& gpu = GPUResources::Get();
                auto buffers = gpu.GetAllBuffers();

                ImGui::BeginTable("Buffers", 2);
                ImGui::TableSetupColumn("UUID", ImGuiTableColumnFlags_NoResize | ImGuiTableColumnFlags_WidthStretch);
                ImGui::TableSetupColumn("Size", ImGuiTableColumnFlags_IndentDisable | ImGuiTableColumnFlags_WidthFixed);

                ImGui::TableHeadersRow();

                ImGui::TableNextColumn();

                for (auto& buffer : buffers)
                {
                    ImGui::Text(buffer->GetName().c_str());

                    ImGui::TableNextColumn();
                     
                    ImGui::Text(std::to_string(buffer->GetSize()).c_str());
                    ImGui::TableNextColumn();
                }

                ImGui::EndTable();
            }
            ImGui::End();
        }

        if (m_ShowMapImporter)
        {
            m_MapImporter.Draw();
        }

        ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDocking;

        ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->Pos);
        ImGui::SetNextWindowSize(viewport->Size);
        ImGui::SetNextWindowViewport(viewport->ID);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
        window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_NoScrollbar;

        const bool isMaximized = Window::Get()->IsMaximized();

        auto projCol = Engine::GetProject()->Settings.PrimaryColor;
        ImVec4 col = ImVec4{ projCol.x, projCol.g, projCol.b, 1.0 };
        ImGui::PushStyleColor(ImGuiCol_WindowBg, col);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, isMaximized ? ImVec2(6.0f, 8.0f) : ImVec2(1.0f, 1.0f));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 3.0f);

       
        ImGui::PushStyleColor(ImGuiCol_MenuBarBg, UI::PrimaryCol);
        ImGui::Begin("DockSpaceWindow22", nullptr, window_flags);
        ImGui::PopStyleColor(); // MenuBarBg
        ImGui::PopStyleColor(); // windowbg
        ImGui::PopStyleVar(2);

        ImGui::PopStyleVar(2);

        {
            ImGui::PushStyleColor(ImGuiCol_Border, IM_COL32(50, 50, 50, 255));
            // Draw window border if the window is not maximized
            if (!isMaximized)
            {
                struct ImGuiResizeBorderDef
                {
                    ImVec2 InnerDir;
                    ImVec2 SegmentN1, SegmentN2;
                    float  OuterAngle;
                };

                static const ImGuiResizeBorderDef resize_border_def[4] =
                {
                    { ImVec2(+1, 0), ImVec2(0, 1), ImVec2(0, 0), IM_PI * 1.00f }, // Left
                    { ImVec2(-1, 0), ImVec2(1, 0), ImVec2(1, 1), IM_PI * 0.00f }, // Right
                    { ImVec2(0, +1), ImVec2(0, 0), ImVec2(1, 0), IM_PI * 1.50f }, // Up
                    { ImVec2(0, -1), ImVec2(1, 1), ImVec2(0, 1), IM_PI * 0.50f }  // Down
                };

                auto GetResizeBorderRect = [](ImGuiWindow* window, int border_n, float perp_padding, float thickness)
                    {
                        ImRect rect = window->Rect();
                        if (thickness == 0.0f)
                        {
                            rect.Max.x -= 1;
                            rect.Max.y -= 1;
                        }
                        if (border_n == ImGuiDir_Left) { return ImRect(rect.Min.x - thickness, rect.Min.y + perp_padding, rect.Min.x + thickness, rect.Max.y - perp_padding); }
                        if (border_n == ImGuiDir_Right) { return ImRect(rect.Max.x - thickness, rect.Min.y + perp_padding, rect.Max.x + thickness, rect.Max.y - perp_padding); }
                        if (border_n == ImGuiDir_Up) { return ImRect(rect.Min.x + perp_padding, rect.Min.y - thickness, rect.Max.x - perp_padding, rect.Min.y + thickness); }
                        if (border_n == ImGuiDir_Down) { return ImRect(rect.Min.x + perp_padding, rect.Max.y - thickness, rect.Max.x - perp_padding, rect.Max.y + thickness); }
                        IM_ASSERT(0);
                        return ImRect();
                    };


                ImGuiContext& g = *GImGui;
                auto window = ImGui::GetCurrentWindow();
                float rounding = window->WindowRounding;
                float border_size = 1.0f; // window->WindowBorderSize;
                if (border_size > 0.0f && !(window->Flags & ImGuiWindowFlags_NoBackground))
                    window->DrawList->AddRect(window->Pos, { window->Pos.x + window->Size.x,  window->Pos.y + window->Size.y }, ImGui::GetColorU32(ImGuiCol_Border), rounding, 0, border_size);

                int border_held = window->ResizeBorderHeld;
                if (border_held != -1)
                {
                    const ImGuiResizeBorderDef& def = resize_border_def[border_held];
                    ImRect border_r = GetResizeBorderRect(window, border_held, rounding, 0.0f);
                    ImVec2 p1 = ImLerp(border_r.Min, border_r.Max, def.SegmentN1);
                    const float offsetX = def.InnerDir.x * rounding;
                    const float offsetY = def.InnerDir.y * rounding;
                    p1.x += 0.5f + offsetX;
                    p1.y += 0.5f + offsetY;

                    ImVec2 p2 = ImLerp(border_r.Min, border_r.Max, def.SegmentN2);
                    p2.x += 0.5f + offsetX;
                    p2.y += 0.5f + offsetY;

                    window->DrawList->PathArcTo(p1, rounding, def.OuterAngle - IM_PI * 0.25f, def.OuterAngle);
                    window->DrawList->PathArcTo(p2, rounding, def.OuterAngle, def.OuterAngle + IM_PI * 0.25f);
                    window->DrawList->PathStroke(ImGui::GetColorU32(ImGuiCol_SeparatorActive), 0, ImMax(2.0f, border_size)); // Thicker than usual
                }
                if (g.Style.FrameBorderSize > 0 && !(window->Flags & ImGuiWindowFlags_NoTitleBar) && !window->DockIsActive)
                {
                    float y = window->Pos.y + window->TitleBarHeight() - 1;
                    window->DrawList->AddLine(ImVec2(window->Pos.x + border_size, y), ImVec2(window->Pos.x + window->Size.x - border_size, y), ImGui::GetColorU32(ImGuiCol_Border), g.Style.FrameBorderSize);
                }
            }

            ImGui::PopStyleColor(); // ImGuiCol_Border
        }

        float titleBarHeight;
        DrawTitlebar(titleBarHeight);
        ImGui::SetCursorPosY(titleBarHeight);

        ImGuiIO& io = ImGui::GetIO();
        ImGuiStyle& style = ImGui::GetStyle();
        float minWinSizeX = style.WindowMinSize.x;
        style.WindowMinSize.x = 370.0f;

        ImGuiWindowClass top_level_class;
        top_level_class.ClassId = ImHashStr("SceneEditor");
        top_level_class.DockingAllowUnclassed = true;

        ImGuiDockNodeFlags flags = ImGuiDockNodeFlags_NoSplit | ImGuiDockNodeFlags_NoResize;
        ImGuiID dockspaceNodeId = ImGui::GetID("SceneEditorDockSpace");
        ImVec2 dockSize = ImGui::GetContentRegionAvail();
        
        ImGuiID dockspaceId = ImGui::DockSpace(dockspaceNodeId, dockSize, flags, &top_level_class);

        if (ImGui::DockBuilderGetNode(SceneEditorDockspaceNodeID) == 0)
        {
            SceneEditorDockspaceNodeID = ImGui::DockBuilderAddNode(dockspaceId, ImGuiDockNodeFlags_DockSpace);
        }

        // We need to cache, because we might delete one while iterating
        auto cachedEditors = sceneEditors;
        for (auto& sceneEditor : cachedEditors)
        {
            sceneEditor->Draw();
        }
        //ImGuiID node2 = ImGui::DockBuilderAddNode(dockspaceNodeId, ImGuiDockNodeFlags_DockSpace);
        //ImGui::DockBuilderSetNodeSize(node2, dockSize);

        ImGuiDockNode* node = (ImGuiDockNode*)GImGui->DockContext.Nodes.GetVoidPtr(ImGui::GetID("SceneEditorDockSpace"));
        if (node)
        {
            ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 32);
            if (ImGui::DockNodeBeginAmendTabBar(node))
            {
                ImGui::SetNextItemWidth(48);
                if (ImGui::BeginTabItem("##logoPadding", 0, ImGuiTabItemFlags_Leading))
                {

                    ImGui::EndTabItem();
                }
                ImGui::DockNodeEndAmendTabBar();
            }
        }

        ImGui::End();



        //DrawMenuBar();
        //DrawMenuBars();

        int i = 0;
        if (Logger::GetLogCount() > 0 && Logger::GetLogs()[Logger::GetLogCount() - 1].type == COMPILATION)
        {
            SetStatusMessage(std::string(ICON_FA_EXCLAMATION_TRIANGLE) + "  An unhandled exception occured in your script. See logs for more details.", Color(1.0f, 0.1f, 0.1f, 1.0f));
        }

        DrawStatusBar();
        
        for (auto& prefabEditors : prefabEditors)
        {
            prefabEditors->Draw();
        }



		//pInterface.DrawEntitySettings();
        //DrawViewport();
        //DrawSceneTree();
        //SelectionPanel->Draw(Selection);
        //DrawLogger();
        //
        //if (this->showFloatingFileBrowser)
        //{
        //    filesystem->Draw();
        //    filesystem->DrawDirectoryExplorer();
        //}

        //auto node = ImGui::DockBuilderGetNode(1);
        //node->SizeRef = { node->Size.x, 50.0f };

        if (isNewProject)
        {
            ImVec2 center = ImGui::GetMainViewport()->GetCenter();
            ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
            if (ImGui::BeginPopupModal("Welcome to Nuake Engine!", 0, ImGuiWindowFlags_AlwaysAutoResize))
            {
                ImGui::TextWrapped("If you would like to use the Trenchbroom integration, please locate your Trenchbroom executable.");
        
                if (ImGui::Button("Locate..."))
                {
                    const std::string& locationPath = Nuake::FileDialog::OpenFile("TrenchBroom (.exe)\0TrenchBroom.exe\0");
        
                    if (!locationPath.empty())
                    {
                        Engine::GetProject()->TrenchbroomPath = locationPath;
                    }
                }
                ImGui::SameLine();
                ImGui::InputText("##Trenchbroom Path", &Engine::GetProject()->TrenchbroomPath);
                ImGui::TextColored(ImVec4(1, 1, 1, 0.5), "Note: You can configure this later in the Project Settings");
                ImGui::Text("");
                if (ImGui::Button("OK"))
                {
                    isNewProject = false;
                    Engine::GetProject()->Save();
                }
        
                ImGui::EndPopup();
            }
        
            PopupHelper::OpenPopup("Welcome to Nuake Engine!");
        }
        
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
       // isControllingCamera = editorCam->Update(ts, m_IsHoveringViewport && m_IsViewportFocused);

        const bool entityIsSelected = Selection.Type == EditorSelectionType::Entity && Selection.Entity.IsValid();
        if (entityIsSelected && Input::IsKeyPressed(Key::F))
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

        for (auto& sceneEditors : sceneEditors)
        {
            sceneEditors->Update(ts);
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
            return entityTypeName;
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
        const bool isProjectLoaded = Engine::GetProject() != nullptr;
        for (const auto& path : paths)
        {
            if (isProjectLoaded)
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
            else
            {
                _WelcomeWindow->ImportProject(path);
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

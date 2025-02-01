
#include "Window.h"

#include "Engine.h"
#include "Nuake/Resource/Project.h"
#include "Nuake/Core/Maths.h"
#include "Nuake/Rendering/Renderer.h"
#include "Nuake/Rendering/Buffers/Framebuffer.h"
#include "Nuake/Scene/Scene.h"
#include "Nuake/Rendering/Vulkan/VulkanRenderer.h"
#include "Nuake/Resource/StaticResources.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>
#include <Tracy.hpp>

using namespace Nuake;

Window::Window() :
    title(DEFAULT_TITLE),
    width(DEFAULT_WIDTH),
    height(DEFAULT_HEIGHT)
{
    Init();
    Renderer::Init();
}

Ref<Window> Window::Get()
{
    static Ref<Window> instance;
    if (instance == nullptr)
    {
        instance = CreateRef<Window>();
    }

    return instance;
}

GLFWwindow* Window::GetHandle()
{
    return this->window;
}

bool Window::ShouldClose()
{
    return glfwWindowShouldClose(this->window);
}

int Window::Init()
{
    if (!glfwInit())
    {
        Logger::Log("GLFW initialization failed", "window", CRITICAL);
        return -1;
    }

    ShowTitleBar(false);

#ifdef NK_VK
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
#endif 

    this->window = glfwCreateWindow(width, height, title.c_str(), NULL, NULL);
    if (!this->window)
    {
        Logger::Log("Window creation failed", "window", CRITICAL);
        return -1;
    }
    
    SetWindowIcon("resources/Images/nuake-logo.png");
    glfwMakeContextCurrent(this->window);
    //SetVSync(false);

#ifndef NK_VK

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        Logger::Log("glad initialization failed!", "window", CRITICAL);
        return -1;
    }

    Logger::Log("Driver detected " + std::string(((char*)glGetString(GL_VERSION))), "window");

#endif

    //if (glfwRawMouseMotionSupported())
    //    glfwSetInputMode(this->window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
    //
    //// TODO: Move this to renderer init. The window shouldnt have to do gl calls.
    //glfwWindowHint(GLFW_SAMPLES, 4);
    //glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    //glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
    //glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    glfwSetWindowUserPointer(window, this);
    
    glfwSetWindowCloseCallback(window, [](GLFWwindow* nativeWindow) 
        {
            Window* window = reinterpret_cast<Window*>(glfwGetWindowUserPointer(nativeWindow));
            window->OnWindowClosed(*window);
        });
    
    glfwSetWindowFocusCallback(window, [](GLFWwindow* nativeWindow, int focused)
        {
            Window* window = reinterpret_cast<Window*>(glfwGetWindowUserPointer(nativeWindow));
            window->OnWindowFocused(*window, static_cast<bool>(focused));
        });
    
    glfwSetDropCallback(window, [](GLFWwindow* nativeWindow, int count, const char** paths)
        {
            std::vector<std::string> filePaths;
            filePaths.reserve(count);
    
            int i;
            for (i = 0; i < count; i++)
            {
                std::string filePath = std::string(paths[i]);
                filePaths.push_back(filePath);
            }
    
            Window* window = reinterpret_cast<Window*>(glfwGetWindowUserPointer(nativeWindow));
            window->OnDragNDropCallback(*window, filePaths);
        });
    
    glfwSetTitlebarHitTestCallback(window, [](GLFWwindow* nativeWindow, int x, int y, int* hit)
    {
        bool isHit = false;
        Window* window = reinterpret_cast<Window*>(glfwGetWindowUserPointer(nativeWindow));
    
        window->TitlebarHitTest(*window, x, y, isHit);
    
        *hit = isHit;
    });

#ifndef  NK_VK
    // TODO: have clear color in environnement.
    glClearColor(0.f, 0.f, 0.f, 1.0f);

    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_MULTISAMPLE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

#endif //  NK_VK
    const Vector2 defaultResolution = Vector2(1, 1);
    this->framebuffer = CreateRef<FrameBuffer>(true, defaultResolution);

    Ref<Texture> outputTexture = CreateRef<Texture>(defaultResolution, GL_RGB);
    outputTexture->SetParameter(GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    outputTexture->SetParameter(GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    this->framebuffer->SetTexture(outputTexture);
    this->framebuffer->SetTexture(CreateRef<Texture>(defaultResolution, GL_RED_INTEGER, GL_R32I, GL_INT), GL_COLOR_ATTACHMENT1); // Entity ID


    //glEnable(GL_CULL_FACE);

    // Create viewports
    //InitImgui();

    //Logger::Log("ImGui initialized ", "renderer");
    return 0;
}

void Window::Update(Timestep ts)
{
    this->scene->Update(ts);
}

void Window::FixedUpdate(Timestep ts)
{
    this->scene->FixedUpdate(ts);
}

void Window::Draw()
{
    ZoneScoped;

    // Dont render if no scene is loaded.
    if (!this->scene)
    {
        return;
    }

    // Dont render if no cam is found.
    Ref<Camera> cam = this->scene->GetCurrentCamera();
    if (!cam)
    {
        return;
    }

    // We cannot render to a framebuffer that is smaller than 1x1.
    Vector2 size = this->framebuffer->GetSize();
    size.x = std::max(size.x, 1.0f);
    size.y = std::max(size.y, 1.0f);

    cam->AspectRatio = size.x / size.y;
}

void Window::EndDraw()
{
    {
        ZoneScopedN("ImGui::EndFrame");
        ImGui::EndFrame();
    }

    {
        ZoneScopedN("ImGui::Render");
        ImGui::Render();
    }

    auto& vkRenderer = VkRenderer::Get();
    GPUResources::Get().RecreateBindlessTextures();
    if(vkRenderer.Draw())
    {
        // Render whatever we want in here :)
        if (auto scene = GetScene(); scene != nullptr)
        {
            RenderContext ctx
            {
                scene,
                vkRenderer.GetCurrentCmdBuffer(),
                Vector2{},
				scene->GetCurrentCamera()->ID
            };

            vkRenderer.PrepareSceneData(ctx);
            //vkRenderer.DrawScene(ctx);
            vkRenderer.DrawScenes();
        }
    }
    vkRenderer.EndDraw();

    //ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        GLFWwindow* backup_current_context = glfwGetCurrentContext();
        //ImGui::UpdatePlatformWindows();
        //ImGui::RenderPlatformWindowsDefault();
        glfwMakeContextCurrent(backup_current_context);
    }

    {
        ZoneScopedN("SwapBuffers");
        glfwSwapBuffers(this->window);
    }

    ZoneScopedN("glfwPollEvents");
    glfwPollEvents();
}

Ref<FrameBuffer> Window::GetFrameBuffer() const
{
    return this->framebuffer;
}

Vector2 Window::GetSize() const
{
    int w, h = 0;
    glfwGetWindowSize(this->window, &w, &h);
    return Vector2(w, h);
}

void Window::SetSize(const Vector2& size)
{
    this->width = static_cast<int32_t>(size.x);
    this->height = static_cast<int32_t>(size.y);
    glfwSetWindowSize(this->window, this->width, this->height);
}

void Window::SetPosition(const Vector2& position)
{
    this->position = position;
    glfwSetWindowPos(this->window, static_cast<int>(position.x), static_cast<int>(position.y));
}

void Window::SetMonitor(int monitorIdx)
{
    int monitorCount;
    GLFWmonitor** monitors = glfwGetMonitors(&monitorCount);

    if (monitorIdx <= monitorCount)
    {
        const auto monitor = *(monitors + monitorIdx);
        const GLFWvidmode* mode = glfwGetVideoMode(monitor);

        int monitorX, monitorY;
        glfwGetMonitorPos(monitor, &monitorX, &monitorY);

        // Calculate the center of the monitor
        int monitorWidth, monitorHeight;
        glfwGetMonitorPhysicalSize(monitor, &monitorWidth, &monitorHeight);

        // Convert physical size to pixels (assuming a DPI scale factor of 1.0)
        monitorWidth = static_cast<int>(monitorWidth * 96.0f / 25.4f);
        monitorHeight = static_cast<int>(monitorHeight * 96.0f / 25.4f);

        // Center the window on the new monitor
        int windowX = monitorX + (monitorWidth - this->width) / 2;
        int windowY = monitorY + (monitorHeight - this->height) / 2;

        glfwSetWindowMonitor(this->window, nullptr, windowX, windowY, this->width, this->height, GLFW_DONT_CARE);
    }
}

void Window::Center()
{
    const int windowWidth = 640;
    const int windowHeight = 360;

    // Get the primary monitor's video mode
    const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());

    // Calculate the position to center the window
    const int xPos = (mode->width - this->width) / 2;
    const int yPos = (mode->height - this->height) / 2;

    // Set the window's position
    Nuake::Window::Get()->SetSize({ this->width, this->height });
    Nuake::Window::Get()->SetPosition({ xPos, yPos });
}

Ref<Scene> Window::GetScene()
{
    return this->scene;
}

bool Window::SetScene(Ref<Scene> newScene)
{
    //windowSetSceneDelegate.Broadcast(scene, newScene);
    
    scene = newScene;
    
    return true;
}

void Window::SetTitle(const std::string& title)
{
    this->title = title;
    glfwSetWindowTitle(this->window, this->title.c_str());
}

std::string Window::GetTitle()
{
    return this->title;
}

void Window::ShowTitleBar(bool visible)
{
    glfwWindowHint(GLFW_TITLEBAR, visible);
}

void Window::SetWindowIcon(const std::string& path)
{
    GLFWimage images[1];
    images[0].pixels = stbi_load_from_memory(StaticResources::Data_Images_editor_icon_png, StaticResources::Data_Images_editor_icon_png_len, &images[0].width, &images[0].height, 0, 4);
    glfwSetWindowIcon(this->window, 1, images);
    stbi_image_free(images[0].pixels);
}

void Window::SetVSync(bool enabled)
{
    glfwSwapInterval(enabled ? 1 : 0);
}

void Window::SetDecorated(bool enabled)
{
    glfwSetWindowAttrib(this->window, GLFW_DECORATED, enabled);
}

void Window::SetFullScreen(bool enabled)
{
    const auto monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(monitor);
    glfwSetWindowMonitor(this->window, monitor, 0, 0, mode->width, mode->height, GLFW_DONT_CARE);
}

void Window::Maximize()
{
    const auto monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(monitor);
    SetSize({mode->width, mode->height});
    Center();
    glfwMaximizeWindow(this->window);
}

bool Window::IsMaximized()
{
    return (bool)glfwGetWindowAttrib(window, GLFW_MAXIMIZED);
}

void Window::SetOnWindowFocusedCallback(std::function<void(Window& window, bool focused)> callback)
{
    onWindowFocusedCallback = callback;
}

void Window::SetOnWindowClosedCallback(std::function<void(Window& window)> callback)
{
    onWindowClosedCallback = callback;
}

void Window::SetOnDragNDropCallback(std::function<void(Window&, const std::vector<std::string>& paths)> callback)
{
    onDragNDropCallback = callback;
}

void Window::SetTitlebarHitTestCallback(std::function<void(Window&, int x, int y, bool&hit)> callback)
{
    titleBarHitTestCallback = callback;
}

void Window::OnWindowFocused(Window& window, bool focused)
{
    if (onWindowFocusedCallback)
    {
        onWindowFocusedCallback(window, focused);
    }
}

void Window::OnWindowClosed(Window& window)
{
    if (onWindowClosedCallback)
    {
        onWindowClosedCallback(window);
    }
}

void Window::OnDragNDropCallback(Window& window, const std::vector<std::string>& paths)
{
    if (onDragNDropCallback)
    {
        onDragNDropCallback(window, paths);
    }
}

void Window::TitlebarHitTest(Window& window, int x, int y, bool& hit)
{
    if (titleBarHitTestCallback)
    {
        titleBarHitTestCallback(window, x, y, hit);
    }
}

void Window::InitImgui()
{
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.Fonts->AddFontFromMemoryTTF(StaticResources::Data_Fonts_Poppins_Regular_ttf, StaticResources::Data_Fonts_Poppins_Regular_ttf_len, 16.0);

    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
    ImGui::StyleColorsDark();

    ImGuiStyle& s = ImGui::GetStyle();
    s.WindowMenuButtonPosition = ImGuiDir_None;
    s.GrabRounding = 2.0f;
    s.CellPadding = ImVec2(8, 8);
    s.WindowPadding = ImVec2(4, 4);
    s.ScrollbarRounding = 9.0f;
    s.ScrollbarSize = 15.0f;
    s.GrabMinSize = 32.0f;
    s.TabRounding = 0;
    s.WindowRounding = 4.0f;
    s.ChildRounding = 4.0f;
    s.FrameRounding = 4.0f;
    s.GrabRounding = 0;
    s.FramePadding = ImVec2(8, 4);
    s.ItemSpacing = ImVec2(8, 4);
    s.ItemInnerSpacing = ImVec2(4, 4);
    s.TabRounding = 4.0f;
    s.WindowBorderSize = 0.0f;
    s.IndentSpacing = 12.0f;
    s.ChildBorderSize = 0.0f;
    s.PopupRounding = 4.0f;
    s.FrameBorderSize = 0.0f;

    ImVec4* colors = ImGui::GetStyle().Colors;
    colors[ImGuiCol_Text] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
    colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
    colors[ImGuiCol_WindowBg] = ImVec4(0.148f, 0.148f, 0.148f, 1.00f);
    colors[ImGuiCol_ChildBg] = ImVec4(0.15f, 0.15f, 0.15f, 1.00f);
    colors[ImGuiCol_PopupBg] = ImVec4(0.08f, 0.08f, 0.08f, 0.94f);
    colors[ImGuiCol_Border] = ImVec4(0.11f, 0.11f, 0.11f, 1.00f);
    colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_FrameBg] = ImVec4(0.08f, 0.08f, 0.08f, 1.00f);
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.09f, 0.09f, 0.09f, 1.00f);
    colors[ImGuiCol_FrameBgActive] = ImVec4(0.13f, 0.13f, 0.13f, 1.00f);
    colors[ImGuiCol_TitleBg] = ImVec4(0.08f, 0.08f, 0.08f, 1.00f);
    colors[ImGuiCol_TitleBgActive] = ImVec4(0.078f, 0.078f, 0.078f, 1.00f);
    colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.00f, 0.00f, 0.00f, 0.51f);
    colors[ImGuiCol_MenuBarBg] = ImVec4(0.08f, 0.08f, 0.08f, 1.00f);
    colors[ImGuiCol_ScrollbarBg] = ImVec4(0.08f, 0.08f, 0.08f, 0.00f);
    colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.34f, 0.34f, 0.34f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.29f, 0.29f, 0.29f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.16f, 0.16f, 0.16f, 1.00f);
    colors[ImGuiCol_CheckMark] = ImVec4(0.55f, 0.76f, 0.29f, 1.00f);
    colors[ImGuiCol_SliderGrab] = ImVec4(0.55f, 0.76f, 0.29f, 1.00f);
    colors[ImGuiCol_SliderGrabActive] = ImVec4(0.55f, 0.76f, 0.29f, 1.00f);
    colors[ImGuiCol_Button] = ImVec4(0.22f, 0.22f, 0.22f, 1.00f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.19f, 0.19f, 0.19f, 1.00f);
    colors[ImGuiCol_ButtonActive] = ImVec4(0.16f, 0.16f, 0.16f, 1.00f);
    colors[ImGuiCol_Header] = ImVec4(0.18f, 0.18f, 0.18f, 1.00f);
    colors[ImGuiCol_HeaderHovered] = ImVec4(0.16f, 0.16f, 0.16f, 1.00f);
    colors[ImGuiCol_HeaderActive] = ImVec4(0.12f, 0.12f, 0.12f, 1.00f);
    colors[ImGuiCol_Separator] = ImVec4(0.08f, 0.08f, 0.08f, 1.00f);
    colors[ImGuiCol_SeparatorHovered] = ImVec4(0.10f, 0.10f, 0.10f, 0.78f);
    colors[ImGuiCol_SeparatorActive] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
    colors[ImGuiCol_ResizeGrip] = ImVec4(0.26f, 0.59f, 0.98f, 0.00f);
    colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.00f);
    colors[ImGuiCol_ResizeGripActive] = ImVec4(0.26f, 0.59f, 0.98f, 0.00f);
    colors[ImGuiCol_Tab] = ImVec4(0.08f, 0.08f, 0.08f, 1.00f);
    colors[ImGuiCol_TabHovered] = ImVec4(0.19f, 0.19f, 0.19f, 1.00f);
    colors[ImGuiCol_TabActive] = ImVec4(0.15f, 0.15f, 0.15f, 1.00f);
    colors[ImGuiCol_TabUnfocused] = ImVec4(0.08f, 0.08f, 0.08f, 1.00f);
    colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.15f, 0.15f, 0.15f, 1.00f);
    colors[ImGuiCol_DockingPreview] = ImVec4(0.55f, 0.76f, 0.29f, 1.00f);
    colors[ImGuiCol_DockingEmptyBg] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
    colors[ImGuiCol_PlotLines] = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
    colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
    colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
    colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
    colors[ImGuiCol_Border] = ImVec4(0.078f, 0.078f, 0.078f, 1.00f);
    colors[ImGuiCol_TableHeaderBg] = ImVec4(0.19f, 0.19f, 0.19f, 1.00f);
    colors[ImGuiCol_TableBorderStrong] = ImVec4(0.11f, 0.11f, 0.11f, 1.00f);
    colors[ImGuiCol_TableBorderLight] = ImVec4(0.11f, 0.11f, 0.11f, 1.00f);
    colors[ImGuiCol_TableRowBg] = ImVec4(0.11f, 0.11f, 0.11f, 1.00f);
    colors[ImGuiCol_TableRowBgAlt] = ImVec4(0.93f, 0.27f, 0.27f, 0.00f);
    colors[ImGuiCol_TextSelectedBg] = ImVec4(0.08f, 0.49f, 0.97f, 0.28f);
    colors[ImGuiCol_DragDropTarget] = ImVec4(0.55f, 0.76f, 0.29f, 1.00f);
    colors[ImGuiCol_NavHighlight] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
    colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
    colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);

    ImGui_ImplGlfw_InitForOpenGL(this->window, true);
    ImGui_ImplOpenGL3_Init("#version 330");
    
}


#include "Window.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "Engine.h"
#include "src/Core/Maths.h"
#include "src/Rendering/Renderer.h"
#include "src/Rendering/Buffers/Framebuffer.h"
#include "src/Scene/Scene.h"

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>

namespace Nuake 
{
    Window::Window()
    {
        m_Title = DEFAULT_TITLE;
        m_Width = DEFAULT_WIDTH;
        m_Height = DEFAULT_HEIGHT;

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
        return m_Window;
    }

    bool Window::ShouldClose()
    {
        return glfwWindowShouldClose(m_Window);
    }

    int Window::Init()
    {
        if (!glfwInit())
        {
            Logger::Log("GLFW initialization failed", "window", CRITICAL);
            return -1;
        }

        m_Window = glfwCreateWindow(m_Width, m_Height, m_Title.c_str(), NULL, NULL);
        if (!m_Window)
        {
            Logger::Log("Window creation failed", "window", CRITICAL);
            return -1;
        }

        SetWindowIcon("resources/Images/nuake-logo.png");

        glfwMakeContextCurrent(m_Window);
        SetVSync(false);

        Logger::Log("Driver detected " + std::string(((char*)glGetString(GL_VERSION))), "renderer");

        if (glewInit() != GLEW_OK)
        {
            Logger::Log("GLEW initialization failed!", "window", CRITICAL);
            return -1;
        }

        if (glfwRawMouseMotionSupported())
            glfwSetInputMode(m_Window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);

        // TODO: Move this to renderer init. The window shouldnt have to do gl calls.
        glfwWindowHint(GLFW_SAMPLES, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

        // TODO: have clear color in environnement.
        glClearColor(0.f, 0.f, 0.f, 1.0f);

        glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_MULTISAMPLE);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        //glEnable(GL_CULL_FACE);

        // Create viewports
        m_Framebuffer = CreateRef<FrameBuffer>(true, glm::vec2(1920, 1080));
        m_Framebuffer->SetTexture(CreateRef<Texture>(glm::vec2(1920, 1080), GL_RGB));

        InitImgui();

        return 0;
    }

    void Window::Update(Timestep ts)
    {
        m_Scene->Update(ts);
    }

    void Window::FixedUpdate(Timestep ts)
    {
        m_Scene->FixedUpdate(ts);
    }

    void Window::Draw()
    {
        // Dont render if no scene is loaded.
        if (!m_Scene)
        {
            return;
        }

        // Dont render if no cam is found.
        Ref<Camera> cam = m_Scene->GetCurrentCamera();
        if (!cam)
        {
            return;
        }

        // We cannot render to a framebuffer that is smaller than 1x1.
        Vector2 size = m_Framebuffer->GetSize();
        size.x = std::max(size.x, 1.0f);
        size.y = std::max(size.y, 1.0f);

        cam->AspectRatio = size.x / size.y;

        m_Scene->m_EditorCamera->OnWindowResize(size.x, size.y);

        if (Engine::IsPlayMode())
        {
            m_Scene->Draw(*m_Framebuffer.get());
        }
        else
        {
            m_Scene->Draw(*m_Framebuffer.get(), m_Scene->m_EditorCamera->GetPerspective(), m_Scene->m_EditorCamera->GetTransform());
        }

        glEnable(GL_DEPTH_TEST);
        Renderer::EndDraw();
    }

    void Window::EndDraw()
    {
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(m_Window);
        glfwPollEvents();
    }

    Ref<FrameBuffer> Window::GetFrameBuffer() const
    {
        return m_Framebuffer;
    }

    Vector2 Window::GetSize() const
    {
        int w, h = 0;
        glfwGetWindowSize(m_Window, &w, &h);
        return Vector2(w, h);
    }

    void Window::SetSize(const Vector2& size)
    {
        m_Width = size.x;
        m_Height = size.y;
        glfwSetWindowSize(m_Window, size.x, size.y);
    }

    void Window::SetPosition(const Vector2& position)
    {
        m_Position = position;
        glfwSetWindowPos(m_Window, position.x, position.y);
    }

    void Window::SetMonitor(int monitorIdx)
    {
        int monitorCount;
        GLFWmonitor** monitors = glfwGetMonitors(&monitorCount);

        if (monitorIdx <= monitorCount)
        {
            const auto monitor = *(monitors + monitorIdx);
            const GLFWvidmode* mode = glfwGetVideoMode(monitor);
            glfwSetWindowMonitor(m_Window, monitor, 0, 0, m_Width, m_Height, mode->refreshRate);
        }
    }

    void Window::Center()
    {
        const int windowWidth = 640;
        const int windowHeight = 360;

        // Get the primary monitor's video mode
        const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());

        // Calculate the position to center the window
        const int xPos = (mode->width - m_Width) / 2;
        const int yPos = (mode->height - m_Height) / 2;

        // Set the window's position
        Nuake::Window::Get()->SetSize({ m_Width, m_Height });
        Nuake::Window::Get()->SetPosition({ xPos, yPos });
    }

    Ref<Scene> Window::GetScene()
    {
        return m_Scene;
    }

    bool Window::SetScene(Ref<Scene> scene)
    {
        m_Scene = scene;
        return true;
    }

    void Window::SetTitle(const std::string& title)
    {
        m_Title = title;
        glfwSetWindowTitle(m_Window, m_Title.c_str());
    }

    std::string Window::GetTitle()
    {
        return m_Title;
    }
    
    void Window::SetWindowIcon(const std::string& path)
    {
        GLFWimage images[1];
        images[0].pixels = stbi_load("resources/Images/nuake-logo.png", &images[0].width, &images[0].height, 0, 4); //rgba channels 
        glfwSetWindowIcon(m_Window, 1, images);
        stbi_image_free(images[0].pixels);
    }

    void Window::SetVSync(bool enabled)
    {
        glfwSwapInterval(enabled ? 1 : 0);
    }

    void Window::SetDecorated(bool enabled)
    {
        glfwSetWindowAttrib(m_Window, GLFW_DECORATED, enabled);
    }

    void Window::InitImgui()
    {
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO(); (void)io;
        io.Fonts->AddFontFromFileTTF("resources/Fonts/Poppins-Regular.ttf", 16.0);

        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
        ImGui::StyleColorsDark();

        ImGuiStyle& s = ImGui::GetStyle();
        s.WindowMenuButtonPosition = ImGuiDir_None;
        s.GrabRounding = 2.0f;
        s.CellPadding = ImVec2(8, 8);
        s.WindowPadding = ImVec2(2, 2);
        s.ScrollbarRounding = 9.0f;
        s.ScrollbarSize = 15.0f;
        s.GrabMinSize = 32.0f;
        s.TabRounding = 0;
        s.WindowRounding = 0;
        s.ChildRounding = 0;
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
        s.FrameBorderSize = 1.0f;

        ImVec4* colors = ImGui::GetStyle().Colors;
        colors[ImGuiCol_Text] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
        colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
        colors[ImGuiCol_WindowBg] = ImVec4(0.08f, 0.08f, 0.08f, 1.00f);
        colors[ImGuiCol_ChildBg] = ImVec4(0.15f, 0.15f, 0.15f, 1.00f);
        colors[ImGuiCol_PopupBg] = ImVec4(0.08f, 0.08f, 0.08f, 0.94f);
        colors[ImGuiCol_Border] = ImVec4(0.11f, 0.11f, 0.11f, 1.00f);
        colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
        colors[ImGuiCol_FrameBg] = ImVec4(0.06f, 0.06f, 0.06f, 1.00f);
        colors[ImGuiCol_FrameBgHovered] = ImVec4(0.09f, 0.09f, 0.09f, 1.00f);
        colors[ImGuiCol_FrameBgActive] = ImVec4(0.13f, 0.13f, 0.13f, 1.00f);
        colors[ImGuiCol_TitleBg] = ImVec4(0.08f, 0.08f, 0.08f, 1.00f);
        colors[ImGuiCol_TitleBgActive] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
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

        ImGui_ImplGlfw_InitForOpenGL(m_Window, true);
        ImGui_ImplOpenGL3_Init("#version 330");
    }
}


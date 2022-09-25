
#include <GL\glew.h>

#include "Window.h"
#include <GLFW/glfw3.h>
#include <iostream>

#include "Rendering/Shaders/Shader.h"
#include <imgui\imgui.h>
#include "Rendering/Renderer.h"
#include "Scene/Entities/Entity.h"
#include <imgui\imgui_impl_glfw.h>
#include <imgui\imgui_impl_opengl3.h>
#include "Scene/Entities/ImGuiHelper.h"
#include "Core/Physics/PhysicsManager.h"
#include "imgui/ImGuizmo.h"
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include "Resource/FontAwesome5.h"
#include "Engine.h"
#include <src/Scene/Components/InterfaceComponent.h>

namespace Nuake {
    Ref<Window> Window::s_Instance;

    Window::Window()
    {
        m_Title = DEFAULT_TITLE;

        Init();
        Renderer::Init();
    }

    Window::~Window() { }

    Ref<Window> Window::Get()
    {
        if (s_Instance == nullptr)
            s_Instance = CreateRef<Window>();

        return s_Instance;
    }

    bool Window::ShouldClose()
    {
        return glfwWindowShouldClose(m_Window);
    }

    GLFWwindow* Window::GetHandle()
    {
        return m_Window;
    }

    bool Window::SetScene(Ref<Scene> scene)
    {
        m_Scene = scene;
        return true;
    }

    Ref<Scene> Window::GetScene()
    {
        return m_Scene;
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

    Ref<FrameBuffer> Window::GetFrameBuffer() const
    {
        return m_Framebuffer;
    }

    Vector2 Window::GetSize()
    {
        int w, h = 0;
        glfwGetWindowSize(m_Window, &w, &h);
        return Vector2(w, h);
    }

    int Window::Init()
    {
        if (!glfwInit())
        {
            Logger::Log("glfw initialization failed.", CRITICAL);
            return -1;
        }

        { // Create window
            m_Window = glfwCreateWindow(m_Width, m_Height, m_Title.c_str(), NULL, NULL);
            if (!m_Window)
            {
                Logger::Log("Window creation failed.", CRITICAL);
                return -1;
            }

            glfwMakeContextCurrent(m_Window);

            Logger::Log((char*)glGetString(GL_VERSION));

            if (glewInit() != GLEW_OK)
            {
                Logger::Log("GLEW initialization failed!", CRITICAL);
                return -1;
            }

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
        }

        // Create viewports
        m_Framebuffer = CreateRef<FrameBuffer>(true, glm::vec2(1920, 1080));
        m_Framebuffer->SetTexture(CreateRef<Texture>(glm::vec2(1920, 1080), GL_RGB));

        // ImGui init
        {
            ImGui::CreateContext();
            ImGuiIO& io = ImGui::GetIO(); (void)io;
            //io.Fonts->AddFontDefault();
            io.Fonts->AddFontFromFileTTF("resources/Fonts/OpenSans-Regular.ttf", 16.0);
           
            io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
            ImGui::StyleColorsDark();

            ImGuiStyle& s = ImGui::GetStyle();
            s.FrameRounding = 2.0f;
            s.GrabRounding = 2.0f;
            s.CellPadding = ImVec2(8, 8);
            s.WindowPadding = ImVec2(8, 8);
            s.ScrollbarRounding = 9.0f;
            s.TabRounding = 0;
            s.WindowRounding = 0;
            s.ChildRounding = 0;
            s.FrameRounding = 0;
            s.GrabRounding = 0;
            s.FramePadding = ImVec2(8, 4);
            s.ItemSpacing = ImVec2(8, 4);
            s.ItemInnerSpacing = ImVec2(4, 4);

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
            colors[ImGuiCol_ScrollbarBg] = ImVec4(0.08f, 0.08f, 0.08f, 1.00f);
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
            return;

        // Dont render if no cam is found.
        Ref<Camera> cam = m_Scene->GetCurrentCamera();
        if (!cam) 
            return;

        Vector2 size = m_Framebuffer->GetSize();
        cam->AspectRatio = size.x / size.y;
        m_Scene->m_EditorCamera->OnWindowResize(size.x, size.y);

        if (Engine::IsPlayMode)
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
}



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

#include "../Engine.h"

// TODO: Use abstraction for vertex buffers
unsigned int vbo;
unsigned int vao;

// TODO: Move to primitive storage.
float vertices[] = {
    1.0f,  1.0f, 0.0f,  1.0f, 1.0f,
    1.0f, -1.0f, 0.0f,   1.0f, 0.0f,
    -1.0f,  1.0f, 0.0f,  0.0f, 1.0f,
     -1.0f, -1.0f, 0.0f,  0.0f, 0.0f,
    -1.0f,  1.0f, 0.0f,  0.0f, 1.0f,
    1.0f, -1.0f, 0.0f,  1.0f, 0.0f
   
    
};


Ref<Window> Window::s_Instance;

Window::Window() 
{
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
    if (scene == nullptr)
        return false;

    m_Scene = scene;
    return true;
}

Ref<Scene> Window::GetScene()
{
    return m_Scene;
}

Ref<FrameBuffer> Window::GetFrameBuffer() const
{
    return m_Framebuffer;
}


int Window::Init() 
{
    if (!glfwInit()) 
    {
       Logger::Log("glfw initialization failed.");
       return -1;
    }
    
    // Create window
    m_Window = glfwCreateWindow(m_Width, m_Height, "Editor - Dev build", NULL, NULL);
    
    if (!m_Window) 
    {
        Logger::Log("Window creation failed.");
        return -1;
    }

    glfwMakeContextCurrent(m_Window);

    Logger::Log((char*)glGetString(GL_VERSION));
    
    if (glewInit() != GLEW_OK) 
    {
        Logger::Log("GLEW initialization failed!");
        return -1;
    }
    
    // TODO: Move this to renderer init. The window shouldnt have to do gl calls.
    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    
    // TODO: have clear color in environnement.
    glClearColor(0.019f, 0.501f, 1.0f, 1.0f);

    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_MULTISAMPLE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    //glEnable(GL_CULL_FACE);

    // Create viewports
    m_Framebuffer = CreateRef<FrameBuffer>(true, glm::vec2(1920, 1080));
    m_Framebuffer->SetTexture(CreateRef<Texture>(glm::vec2(1920, 1080), GL_RGB));
    
    // Temporary quad vbo for deferred.
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 5, (void*)0);
    glEnableVertexAttribArray(0);
    
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 5, (void*)(sizeof(float) * 3));
    glEnableVertexAttribArray(1);
    
    // TODO: Style should move to editor
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.Fonts->AddFontDefault();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    ImGui::StyleColorsDark();

    ImGui::GetStyle().FrameRounding = 4.0f;
    ImGui::GetStyle().GrabRounding = 4.0f;

    ImVec4* colors = ImGui::GetStyle().Colors;
    colors[ImGuiCol_Text] = ImVec4(0.95f, 0.96f, 0.98f, 1.00f);
    colors[ImGuiCol_TextDisabled] = ImVec4(0.36f, 0.42f, 0.47f, 1.00f);
    colors[ImGuiCol_WindowBg] = ImVec4(0.11f, 0.15f, 0.17f, 1.00f);
    colors[ImGuiCol_ChildBg] = ImVec4(0.15f, 0.18f, 0.22f, 1.00f);
    colors[ImGuiCol_PopupBg] = ImVec4(0.08f, 0.08f, 0.08f, 0.94f);
    colors[ImGuiCol_Border] = ImVec4(0.08f, 0.10f, 0.12f, 1.00f);
    colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_FrameBg] = ImVec4(0.20f, 0.25f, 0.29f, 1.00f);
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.12f, 0.20f, 0.28f, 1.00f);
    colors[ImGuiCol_FrameBgActive] = ImVec4(0.09f, 0.12f, 0.14f, 1.00f);
    colors[ImGuiCol_TitleBg] = ImVec4(0.09f, 0.12f, 0.14f, 0.65f);
    colors[ImGuiCol_TitleBgActive] = ImVec4(0.08f, 0.10f, 0.12f, 1.00f);
    colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.00f, 0.00f, 0.00f, 0.51f);
    colors[ImGuiCol_MenuBarBg] = ImVec4(0.15f, 0.18f, 0.22f, 1.00f);
    colors[ImGuiCol_ScrollbarBg] = ImVec4(0.02f, 0.02f, 0.02f, 0.39f);
    colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.20f, 0.25f, 0.29f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.18f, 0.22f, 0.25f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.09f, 0.21f, 0.31f, 1.00f);
    colors[ImGuiCol_CheckMark] = ImVec4(0.28f, 0.56f, 1.00f, 1.00f);
    colors[ImGuiCol_SliderGrab] = ImVec4(0.28f, 0.56f, 1.00f, 1.00f);
    colors[ImGuiCol_SliderGrabActive] = ImVec4(0.37f, 0.61f, 1.00f, 1.00f);
    colors[ImGuiCol_Button] = ImVec4(0.20f, 0.25f, 0.29f, 1.00f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.28f, 0.56f, 1.00f, 1.00f);
    colors[ImGuiCol_ButtonActive] = ImVec4(0.06f, 0.53f, 0.98f, 1.00f);
    colors[ImGuiCol_Header] = ImVec4(0.20f, 0.25f, 0.29f, 0.55f);
    colors[ImGuiCol_HeaderHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.80f);
    colors[ImGuiCol_HeaderActive] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    colors[ImGuiCol_Separator] = ImVec4(0.20f, 0.25f, 0.29f, 1.00f);
    colors[ImGuiCol_SeparatorHovered] = ImVec4(0.10f, 0.40f, 0.75f, 0.78f);
    colors[ImGuiCol_SeparatorActive] = ImVec4(0.10f, 0.40f, 0.75f, 1.00f);
    colors[ImGuiCol_ResizeGrip] = ImVec4(0.26f, 0.59f, 0.98f, 0.25f);
    colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
    colors[ImGuiCol_ResizeGripActive] = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
    colors[ImGuiCol_Tab] = ImVec4(0.11f, 0.15f, 0.17f, 1.00f);
    colors[ImGuiCol_TabHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.80f);
    colors[ImGuiCol_TabActive] = ImVec4(0.20f, 0.25f, 0.29f, 1.00f);
    colors[ImGuiCol_TabUnfocused] = ImVec4(0.11f, 0.15f, 0.17f, 1.00f);
    colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.11f, 0.15f, 0.17f, 1.00f);
    colors[ImGuiCol_PlotLines] = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
    colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
    colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
    colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
    colors[ImGuiCol_TextSelectedBg] = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
    colors[ImGuiCol_DragDropTarget] = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
    colors[ImGuiCol_NavHighlight] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
    colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
    colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);

    ImGui_ImplGlfw_InitForOpenGL(m_Window, true);
    ImGui_ImplOpenGL3_Init("#version 330");
    return 0;
}


void Window::Update(Timestep ts) 
{
     // TODO: have event here?
    m_Scene->Update(ts);
}

void Window::Draw()
{
    // Dont render if no scene is loaded.
    if (!m_Scene)
        return;

    // Dont render if no cam is found.
    Ref<Camera> cam = m_Scene->GetCurrentCamera();
    if (!cam) return;

    Renderer::BeginDraw(cam);
    {

        //glCullFace(GL_FRONT);
        //glCullFace(GL_BACK);
        m_Scene->DrawShadows();

        m_Framebuffer->Bind();
        
        if (Engine::IsPlayMode)
            m_Scene->Draw();
        else
            m_Scene->EditorDraw();

        m_Scene->DrawInterface(m_Framebuffer->GetSize());
        
        m_Framebuffer->Unbind();
    }

    Renderer::EndDraw();
}

void Window::EndDraw()
{
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    glfwSwapBuffers(m_Window);
    glfwPollEvents();
}

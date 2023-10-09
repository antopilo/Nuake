#include <glad/glad.h>

#include <Engine.h>

#include <src/Core/Input.h>
#include <src/Scene/Scene.h>
#include <src/Scene/Entities/Entity.h>
#include <src/Scene/Components/Components.h>

#include "src/Windows/EditorInterface.h"

#include <src/Scene/Components/QuakeMap.h>
#include <src/Vendors/imgui/imgui.h>
#include <src/Vendors/imgui/ImGuizmo.h>
#include <src/Core/Physics/PhysicsManager.h>
#include "src/Scene/Components/BoxCollider.h"

#include <GLFW/glfw3.h>
#include <src/Vendors/glm/trigonometric.hpp>
#include <src/Scripting/ScriptingEngine.h>
#include <src/Resource/FGD/FGDFile.h>
#include <src/Rendering/Shaders/ShaderManager.h>
#include <src/Rendering/Renderer.h>
#include "src/UI/UserInterface.h"
#include "src/NewEditor.h"
#include <src/Scene/Components/BSPBrushComponent.h>

#include "src/Actions/EditorSelection.h"
#include "src/Misc/GizmoDrawer.h"
#include "src/Windows/FileSystemUI.h"

#include <src/Core/Maths.h>
#include <src/Rendering/SceneRenderer.h>

#include "src/Misc/WindowTheming.h"


struct LaunchSettings
{
    int32_t monitor = -1;
    Vector2 resolution = { 1920, 1080 };
    std::string windowTitle = "Nuake Editor ";
    std::string projectPath;
};

std::vector<std::string> ParseArguments(int argc, char* argv[])
{
    std::vector<std::string> args;
    for (uint32_t i = 0; i < argc; i++)
    {
        args.push_back(std::string(argv[i]));
    }
    return args;
}

LaunchSettings ParseLaunchSettings(const std::vector<std::string>& arguments)
{
    LaunchSettings launchSettings;

    const auto argumentSize = arguments.size();
    size_t i = 0;
    for (const auto& arg : arguments)
    {
        const size_t nextArgumentIndex = i + 1;
        const bool containsAnotherArgument = nextArgumentIndex <= argumentSize;
        if (arg == "--project")
        {
            if (!containsAnotherArgument)
            {
                continue;
            }

            // Load project on start
            std::string projectPath = arguments[i + 1];
            launchSettings.projectPath = projectPath;
            
        }
        else if (arg == "--resolution")
        {
            if (!containsAnotherArgument)
            {
                continue;
            }

            // Set editor window resolution
            std::string resString = arguments[i + 1];
            const auto& resSplits = String::Split(resString, 'x');
            if (resSplits.size() == 2)
            {
                int width = stoi(resSplits[0]);
                int height = stoi(resSplits[1]);
                launchSettings.resolution = Vector2(width, height);
            }
        }
        else if (arg == "--monitor")
        {
            // Set editor window monitor
            if (containsAnotherArgument)
            {
                launchSettings.monitor = stoi(arguments[i + 1]);
            }
        }

        i++;
    }

    return launchSettings;
}

int ApplicationMain(int argc, char* argv[])
{
    using namespace Nuake;

    // Parse launch arguments
    const auto& arguments = ParseArguments(argc, argv);
    LaunchSettings launchSettings = ParseLaunchSettings(arguments);

#ifdef NK_DEBUG
    launchSettings.windowTitle += "(DEBUG BUILD)";
#endif // NK_DEBUG

    // Initialize Engine & Window
    Engine::Init();
    auto window = Engine::GetCurrentWindow();
    window->SetSize(launchSettings.resolution);
    window->SetTitle(launchSettings.windowTitle);

    if (launchSettings.monitor >= 0)
    {
        window->SetMonitor(launchSettings.monitor);
    }
    WindowTheming::SetWindowDarkMode(window);

    Logger::Log("Creating editor ", "window", CRITICAL);
    // Initialize Editor
    Nuake::EditorInterface editor;
   
    Logger::Log("Created editor ", "window", CRITICAL);
    // Load project in argument
    if (!launchSettings.projectPath.empty())
    {
        Logger::Log("Loading project", "window", CRITICAL);
        editor.LoadProject(launchSettings.projectPath);
    }

    // Start application main loop
    GizmoDrawer gizmoDrawer = GizmoDrawer(&editor);
    while (!window->ShouldClose())
    {
        Nuake::Engine::Tick(); // Update

        Nuake::Engine::Draw(); // Render
       
        // Render editor
        Nuake::Vector2 WindowSize = window->GetSize();
        glViewport(0, 0, WindowSize.x, WindowSize.y);
        Nuake::Renderer2D::BeginDraw(WindowSize);

        auto sceneFramebuffer = window->GetFrameBuffer();

        Ref<Nuake::Scene> currentScene = Nuake::Engine::GetCurrentScene();

        // Draw gizmos
        sceneFramebuffer->Bind();
        {
            //glDepthMask(false);
            
            
            Ref<EditorCamera> camera;
            if (currentScene)
            { 
                camera = currentScene->m_EditorCamera;
                glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, currentScene->m_SceneRenderer->GetGBuffer().GetTexture(GL_DEPTH_ATTACHMENT)->GetID(), 0);
                //glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, currentScene->m_SceneRenderer->GetGBuffer().GetTexture(GL_COLOR_ATTACHMENT3)->GetID(), 0);
            }

            if (currentScene && !Nuake::Engine::IsPlayMode())
            {
                glEnable(GL_LINE_SMOOTH);

                if (editor.ShouldDrawAxis())
                {
                    //gizmoDrawer.DrawAxis(currentScene);
                }

                if (editor.ShouldDrawCollision())
                {
                    gizmoDrawer.DrawGizmos(currentScene, false);
                    glDepthFunc(GL_GREATER);

                    gizmoDrawer.DrawGizmos(currentScene, true);
                    glDepthFunc(GL_LESS);
                }
            }
            //glDepthMask(true);


        }
        sceneFramebuffer->Unbind();


        // Update & Draw editor
        editor.Draw();
        editor.Update(Nuake::Engine::GetTimestep());
        Nuake::Engine::EndDraw();
    }

    Nuake::Engine::Close();
    return 0;
}



#ifdef NK_WIN
#ifdef NK_DIST
#include "windows.h"
int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hInstPrev, LPSTR cdmline, int cmdshow)
{
    return ApplicationMain(__argc, __argv);
}
#endif
#endif


int main(int argc, char* argv[]) 
{
    return ApplicationMain(argc, argv);
}



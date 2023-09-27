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

    // Initialize Editor
    Nuake::EditorInterface editor;
   
    // Load project in argument
    if (!launchSettings.projectPath.empty())
    {
       editor.LoadProject(launchSettings.projectPath);
    }

    // Start application main loop
    GizmoDrawer gizmoDrawer = GizmoDrawer();
    while (!window->ShouldClose())
    {
        Nuake::Engine::Tick(); // Update
        Nuake::Engine::Draw(); // Render
       
        // Render editor
        Nuake::Vector2 WindowSize = window->GetSize();
        glViewport(0, 0, WindowSize.x, WindowSize.y);
        Nuake::Renderer2D::BeginDraw(WindowSize);

        // Draw gizmos
        auto sceneFramebuffer = window->GetFrameBuffer();
        sceneFramebuffer->Bind();
        {
            Ref<Nuake::Scene> currentScene = Nuake::Engine::GetCurrentScene();
            Ref<EditorCamera> camera;
            if (currentScene)
            {
                camera = currentScene->m_EditorCamera;
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
                    gizmoDrawer.DrawGizmos(currentScene);
                }
            }
        }
        sceneFramebuffer->Unbind();

        // Update & Draw editor
        editor.Update(Nuake::Engine::GetTimestep());
        editor.Draw();

        Nuake::Engine::EndDraw();
    }

    // Shutdown
    Nuake::Engine::Close();
    return 0;
}

#ifdef NK_DIST

#include "windows.h"

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hInstPrev, LPSTR cdmline, int cmdshow)
{
    return ApplicationMain(__argc, __argv);
}

#else

int main(int argc, char* argv[]) 
{
    return ApplicationMain(argc, argv);
}

#endif

#include <glad/glad.h>

#include <Engine.h>

#include <src/Core/Input.h>
#include <src/Scene/Scene.h>
#include <src/Scene/Entities/Entity.h>
#include <src/Scene/Components.h>

#include "src/Windows/EditorInterface.h"

#include <src/Vendors/imgui/imgui.h>
#include <src/Vendors/imgui/ImGuizmo.h>
#include <src/Physics/PhysicsManager.h>

#include <GLFW/glfw3.h>
#include <src/Vendors/glm/trigonometric.hpp>
#include <src/Scripting/ScriptingEngine.h>
#include <src/Resource/FGD/FGDFile.h>
#include <src/Rendering/Shaders/ShaderManager.h>
#include <src/Rendering/Renderer.h>

#include "src/Actions/EditorSelection.h"
#include "src/Misc/GizmoDrawer.h"
#include "src/Windows/FileSystemUI.h"

#include "src/Core/Maths.h"
#include "src/Rendering/SceneRenderer.h"

#include "src/Misc/WindowTheming.h"
#include "src/Application/Application.h"

#include "src/Application/EntryPoint.h"

#include "src/EditorApplication.h"

#include "LaunchSettings.h"


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
        else if (argumentSize == 2 && Nuake::FileSystem::FileExists(arg))
        {
            if (Nuake::String::EndsWith(arg, ".project"))
            {
                launchSettings.projectPath = arg;
            }
        }

        i++;
    }

    return launchSettings;
}


Nuake::Application* Nuake::CreateApplication(int argc, char** argv)
{
    using namespace Nuake;

    const auto& arguments = ParseArguments(argc, argv);
    LaunchSettings launchSettings = ParseLaunchSettings(arguments);

    ApplicationSpecification specification
    {
        .Name = "Editor",
        .WindowWidth = 1100,
        .WindowHeight = 630,
        .VSync = true
    };

#ifdef NK_DEBUG
    specification.Name += "(DEBUG BUILD)";
#endif

    return new EditorApplication(specification, launchSettings);
}

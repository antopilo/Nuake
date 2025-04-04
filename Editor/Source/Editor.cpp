#include <Engine.h>

#include <Nuake/Core/Input.h>
#include <Nuake/Scene/Scene.h>
#include <Nuake/Scene/Entities/Entity.h>
#include <Nuake/Scene/Components.h>

#include "Editor/Windows/EditorInterface.h"

#include "imgui/imgui.h"
#include <imgui/ImGuizmo.h>
#include <Nuake/Physics/PhysicsManager.h>

#include <glm/trigonometric.hpp>
#include <Nuake/Resource/FGD/FGDFile.h>
#include <Nuake/Rendering/Shaders/ShaderManager.h>
#include <Nuake/Rendering/Renderer.h>

#include "Editor/Actions/EditorSelection.h"
#include "Editor/Misc/GizmoDrawer.h"
#include "Editor/Windows/FileSystemUI.h"

#include "Nuake/Core/Maths.h"
#include "Nuake/Rendering/SceneRenderer.h"

#include "Editor/Misc/WindowTheming.h"
#include "Nuake/Application/Application.h"

#include "Nuake/Application/EntryPoint.h"

#include "Editor/EditorApplication.h"

#include "Editor/LaunchSettings.h"


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
        else if (arg == "--generate-bindings")
        {
            // Set editor window monitor
            launchSettings.generateBindings = true;
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
        .WindowWidth = static_cast<uint32_t>(launchSettings.resolution.x),
        .WindowHeight = static_cast<uint32_t>(launchSettings.resolution.y),
        .VSync = true
    };


#ifdef NK_DEBUG
    specification.Name += "(DEBUG BUILD)";
#endif

    return new EditorApplication(specification, launchSettings);
}

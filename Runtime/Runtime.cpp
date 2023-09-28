#include <Engine.h>

//#include <dependencies/GLEW/include/GL/glew.h>
#include <src/Vendors/imgui/imgui.h>

#include <string>

struct LaunchSettings
{
    int32_t monitor = -1;
    Nuake::Vector2 resolution = { 1920, 1080 };
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
    using namespace Nuake;
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

    const auto& arguments = ParseArguments(argc, argv);
    LaunchSettings launchSettings = ParseLaunchSettings(arguments);

    Engine::Init();

    auto window = Nuake::Engine::GetCurrentWindow();

    std::string projectPath; 
    if (launchSettings.projectPath.empty())
    {
        projectPath = "./game.project";
    }
    else
    {
        projectPath = launchSettings.projectPath;
    }
    
    FileSystem::SetRootDirectory(FileSystem::GetParentPath(projectPath));

    Ref<Nuake::Project> project = Nuake::Project::New();
    project->FullPath = projectPath;

    if (!FileSystem::FileExists(projectPath))
    {
        Logger::Log("game.project not found", "runtime", CRITICAL);
        return -1;
    }

    const std::string& projectfileContent = FileSystem::ReadFile(projectPath, true);
    if (projectfileContent.empty())
    {
        Logger::Log("game.project was empty", "runtime", CRITICAL);
        return -1;
    }

    project->Deserialize(json::parse(projectfileContent));

    window->SetTitle(project->Name);

    Nuake::Engine::LoadProject(project);
    Nuake::Engine::EnterPlayMode();

    while (!window->ShouldClose())
    {
        Nuake::Engine::Tick();
        Engine::Draw();

        const auto& WindowSize = window->GetSize();
        //glViewport(0, 0, WindowSize.x, WindowSize.y);
        Nuake::Renderer2D::BeginDraw(WindowSize);

        ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->Pos);
        ImGui::SetNextWindowSize(viewport->Size);
        ImGui::SetNextWindowViewport(viewport->ID);

        const auto& windowSize = Vector2(viewport->Size.x, viewport->Size.y);
        Ref<FrameBuffer> framebuffer = Engine::GetCurrentWindow()->GetFrameBuffer();
        if (framebuffer->GetSize() != windowSize)
        {
            framebuffer->QueueResize(windowSize);
        }

        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 0, 0 });
        ImGui::Begin("Game", 0, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoResize);
        {
            ImGui::Image((void*)Window::Get()->GetFrameBuffer()->GetTexture()->GetID(), ImGui::GetContentRegionAvail(), ImVec2(0, 1), ImVec2(1, 0));
        }
        ImGui::End();

        ImGui::PopStyleVar(2);

        Engine::EndDraw();
    }
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

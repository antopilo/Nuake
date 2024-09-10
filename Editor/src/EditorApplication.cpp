#include "EditorApplication.h"

#include "Engine.h"
#include "Windows/EditorInterface.h"
#include "Misc/GizmoDrawer.h"
#include "EditorLayer.h"

#include <glad/glad.h>

#include "src/UI/NuakeUI.h"
#include "src/UI/UIInputManager.h"

void EditorApplication::OnInit()
{
    using namespace Nuake;

    Engine::Init();
    m_Window = Engine::GetCurrentWindow();
    m_Window->SetSize({ m_Specification.WindowWidth, m_Specification.WindowHeight });
    m_Window->SetTitle(m_Specification.Name);
    //m_Window->SetMonitor(1);
    
    if (!m_LaunchSettings.projectPath.empty())
    {
        if (FileSystem::FileExists(m_LaunchSettings.projectPath, true))
        {
            const std::string projectPath = m_LaunchSettings.projectPath;
            
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
            }
            catch (std::exception exception)
            {
                Logger::Log("Error loading project: " + projectPath, "editor", CRITICAL);
                Logger::Log(exception.what());
            }
        }
    }

    m_Window->SetOnWindowFocusedCallback([&](Window& window, bool focused)
        {
            if (!focused)
            {
                return;
            }

            for (auto& layer : m_LayerStack)
            {
                layer->OnWindowFocused();
            }
        }); 

    m_Window->SetOnWindowClosedCallback([](Window& window)
        {
            if (Engine::GetProject())
            {
                Engine::GetProject()->Save();
            }

            if (Engine::GetCurrentScene())
            {
                Engine::GetCurrentScene()->Save();
            }
        });

    NuakeUI::CanvasParser parser;
    Ref<NuakeUI::Canvas> canvas = parser.Parse("test.html");

    static MyInputManager inputManager(*m_Window);

    canvas->SetInputManager(&inputManager);
    canvas->ComputeLayout({1920, 1080});
    canvas->Tick();
    canvas->Draw();
    
    PushLayer(CreateScope<EditorLayer>());
}

void EditorApplication::OnShutdown()
{
    Nuake::Engine::Close();
}
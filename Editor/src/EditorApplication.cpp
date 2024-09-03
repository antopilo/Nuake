#include "EditorApplication.h"

#include "Engine.h"
#include "Windows/EditorInterface.h"
#include "Misc/GizmoDrawer.h"
#include "EditorLayer.h"

#include <glad/glad.h>


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

    PushLayer(CreateScope<EditorLayer>());
}

void EditorApplication::OnShutdown()
{
    Nuake::Engine::Close();
}
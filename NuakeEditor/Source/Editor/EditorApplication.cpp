#include "EditorApplication.h"

#include "Engine.h"
#include "Windows/EditorInterface.h"
#include "Misc/GizmoDrawer.h"
#include "EditorLayer.h"

#include "Nuake/UI/NuakeUI.h"
#include "Nuake/UI/UIInputManager.h"

#include "Nuake/Resource/Bakers/AssetBakerManager.h"
#include "Nuake/Resource/Bakers/GLTFBaker.h"

void EditorApplication::OnInit()
{
    using namespace Nuake;
    Engine::Init();
	
	// Register bakers, these can convert files into nuake resources
	AssetBakerManager& assetBakerMgr = AssetBakerManager::Get();
	assetBakerMgr.RegisterBaker(CreateRef<GLTFBaker>());
	
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
    
    m_Window->SetOnDragNDropCallback([&](Window& window, const std::vector<std::string>& paths) {
        for (auto& layer : m_LayerStack)
        {
            layer->OnDragNDrop(paths);
        }
    });

    PushLayer(CreateScope<EditorLayer>());
}

void EditorApplication::OnShutdown()
{
    Nuake::Engine::Close();
}
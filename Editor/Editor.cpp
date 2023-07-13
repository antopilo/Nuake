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

const std::string WindowTitle = "Nuake Editor";

int main(int argc, char* argv[])
{
    bool playMode = false;
    std::string projectPath = "";

    Vector2 editorResolution = Vector2(1280, 720);
    int monitorIdx = -1;
    for (uint32_t i = 0; i < argc; i++)
    {
        char* arg = argv[i];
        std::string args = std::string(arg);

        if (args == "--play")
        {
            if (argc > 2)
            {
                projectPath = std::string(argv[i + 1]);
            }
            playMode = true;
        }

        if (args == "--resolution")
        {
            if (argc >= i + 1)
            {
                std::string resString = std::string(argv[i + 1]);
                const auto& resSplits = String::Split(resString, 'x');
                if (resSplits.size() == 2)
                {
                    int width = stoi(resSplits[0]);
                    int height = stoi(resSplits[1]);
                    editorResolution = Vector2(width, height);
                }
            }
        }

        if (args == "--monitor")
        {
            if (argc >= i + 1)
            {
                std::string monitorIdxString = std::string(argv[i + 1]);
                monitorIdx = stoi(monitorIdxString);
               
            }
        }
    }

    bool shouldLoadProject = false;
    if (!playMode && argc > 1)
    {
        shouldLoadProject = true;
        projectPath = std::string(argv[1]);
    }

    if (playMode)
    {
        Nuake::Engine::Init();
        Nuake::EditorInterface editor;
        editor.BuildFonts();

        Ref<Nuake::Project> project = Nuake::Project::New();
        FileSystem::SetRootDirectory(FileSystem::RemoveFileFromPath(projectPath));

        project->FullPath = projectPath;
        project->Deserialize(FileSystem::ReadFile(projectPath, true));

        Ref<Nuake::Window> window = Nuake::Engine::GetCurrentWindow();
        window->SetTitle(project->Name);

        Nuake::Engine::LoadProject(project);

        Nuake::Engine::EnterPlayMode();
        auto shader = Nuake::ShaderManager::GetShader("resources/Shaders/copy.shader");
        while (!window->ShouldClose())
        {
            Nuake::Vector2 WindowSize = window->GetSize();
            glViewport(0, 0, WindowSize.x, WindowSize.y);
            Nuake::Renderer2D::BeginDraw(WindowSize);
            Nuake::Engine::Tick();
            Nuake::Engine::Draw();

            shader->Bind();

            window->GetFrameBuffer()->GetTexture()->Bind(0);
            shader->SetUniform1i("u_Source", 0);
            Nuake::Renderer::DrawQuad(Nuake::Matrix4(1));

            Nuake::Engine::EndDraw();
        }
    }
    else
    {
        Nuake::Engine::Init();
        Engine::GetCurrentWindow()->SetSize(editorResolution);

        Nuake::EditorInterface editor;
        editor.BuildFonts();

        Ref<Nuake::Window> window = Nuake::Engine::GetCurrentWindow();
        window->SetTitle(WindowTitle);

        if (monitorIdx != -1)
        {
            window->SetMonitor(monitorIdx);
        }

        using namespace Nuake;

        GizmoDrawer gizmoDrawer = GizmoDrawer();

        if (shouldLoadProject)
        {
			FileSystem::SetRootDirectory(FileSystem::RemoveFileFromPath(projectPath));

			auto project = Project::New();
			auto projectFileData = FileSystem::ReadFile(projectPath, true);
			try
			{
				project->Deserialize(projectFileData);
				project->FullPath = projectPath;

				Engine::LoadProject(project);

				editor.filesystem->m_CurrentDirectory = Nuake::FileSystem::RootDirectory;
			}
			catch (std::exception exception)
			{
				Logger::Log("Error loading project: " + projectPath, CRITICAL);
				Logger::Log(exception.what());
			}
        }
        
        while (!window->ShouldClose())
        {
            Nuake::Engine::Tick();
            Nuake::Engine::Draw();

            Timestep ts = Nuake::Engine::GetTimestep();

            Nuake::Vector2 WindowSize = window->GetSize();
            glViewport(0, 0, WindowSize.x, WindowSize.y);
            Nuake::Renderer2D::BeginDraw(WindowSize);

            auto sceneFramebuffer = window->GetFrameBuffer();
            sceneFramebuffer->Bind();
            {
                Ref<Nuake::Scene> currentScene = Nuake::Engine::GetCurrentScene();
                Ref<EditorCamera> camera;
                if (currentScene)
                {
                    camera = currentScene->m_EditorCamera;
                }

                if (currentScene && !Nuake::Engine::IsPlayMode() && editor.ShouldDrawAxis())
                {
                    gizmoDrawer.DrawGizmos(currentScene);
                }
            }
            sceneFramebuffer->Unbind();

            editor.Update(ts);
            editor.Draw();

            Nuake::Engine::EndDraw();
        }
    }

    Nuake::Engine::Close();
}
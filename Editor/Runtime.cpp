#include <string>
#include <Engine.h>
#include <dependencies/GLEW/include/GL/glew.h>

#include "src/Rendering/Shaders/ShaderManager.h"
#include "src/Rendering/Renderer.h"
#include <dependencies/glfw/include/GLFW/glfw3.h>
#include <src/Vendors/imgui/imgui.h>
#include <src/Vendors/imgui/imgui_impl_opengl3.h>

void main(int argc, char* argv[])
{
    for (uint32_t i = 0; i < argc; i++)
    {
        char* arg = argv[i];
        std::string projectPath;
        if (argc == 1)
        {
            return;
        }

        projectPath = std::string(argv[1]);

        using namespace Nuake;

        Nuake::Engine::Init();
        Engine::GetCurrentWindow()->SetSize({ 1920, 1080 });

        Ref<Nuake::Window> window = Nuake::Engine::GetCurrentWindow();
        window->SetTitle("Nuake Runtime");

        FileSystem::SetRootDirectory(FileSystem::GetParentPath(projectPath));

        auto project = Project::New();
        auto projectFileData = FileSystem::ReadFile(projectPath, true);
        try
        {
            project->Deserialize(projectFileData);
            project->FullPath = projectPath;

            Engine::LoadProject(project);
        }
        catch (std::exception exception)
        {
            Logger::Log("Error loading project: " + projectPath, "editor", CRITICAL);
            Logger::Log(exception.what());
        }

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        Engine::EnterPlayMode();
        glViewport(0, 0, 100, 100);
        while (!window->ShouldClose())
        {
            Nuake::Engine::Tick();
            Nuake::Engine::Draw();

            Timestep ts = Nuake::Engine::GetTimestep();

           //ImGui::Render();
           //ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

            Nuake::Vector2 WindowSize = window->GetSize();
            glViewport(0, 0, WindowSize.x, WindowSize.y);
            glClear(GL_COLOR_BUFFER_BIT);
            auto shader = ShaderManager::GetShader("resources/Shaders/copy.shader");
            shader->SetUniformTex("u_Source", Engine::GetCurrentScene()->mSceneRenderer->GetTexture().get());
            Renderer::DrawQuad(Matrix4());

            glfwSwapBuffers(window->GetHandle());
            glfwPollEvents();
        }
    }

    Nuake::Engine::Close();
}
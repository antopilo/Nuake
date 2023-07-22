#include <dependencies/GLEW/include/GL/glew.h>
#include <dependencies/glfw/include/GLFW/glfw3.h>


#include <string>
#include <Engine.h>
#include <src/Core/Logger.h>
#include <src/Rendering/SceneRenderer.h>

#include <iostream>
#include <src/Vendors/imgui/imgui.h>

void CheckError()
{
    GLenum err;
    while ((err = glGetError()) != GL_NO_ERROR)
    {
        // Process/log the error.
        std::cout << "OPENGL ERROR: " << err << std::endl;
    }
}

void main(int argc, char* argv[])
{
    std::string projectPath;
    if (argc == 1)
    {
        return;
    }

    projectPath = std::string(argv[1]);

    using namespace Nuake;
    Nuake::Engine::Init();

    Ref<Nuake::Window> window = Nuake::Engine::GetCurrentWindow();
    window->SetTitle("asdasd");

    FileSystem::SetRootDirectory(FileSystem::GetParentPath(projectPath));
    auto project = Project::New();
    auto scene = Scene::New();
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

    //Engine::EnterPlayMode();

    while (!window->ShouldClose())
    {
        Nuake::Engine::Tick();
        Nuake::Engine::Draw();

        ImGui::Begin("test");

        ImGui::End();
        const auto& WindowSize = window->GetSize();
        glViewport(0, 0, WindowSize.x, WindowSize.y);

        RenderCommand::SetClearColor({ 0.1, 0.1 ,0.1, 1.0 });
        RenderCommand::Clear();
        
        Nuake::Engine::EndDraw();
    }
}
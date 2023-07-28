#include <Engine.h>
#include <src/Rendering/SceneRenderer.h>

#include <dependencies/GLEW/include/GL/glew.h>
#include <dependencies/glfw/include/GLFW/glfw3.h>

#include <iostream>
#include <string>

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
    using namespace Nuake;
    Engine::Init();

    auto window = Nuake::Engine::GetCurrentWindow();
    window->SetTitle("asdasd");

    while (!window->ShouldClose())
    {
        Nuake::Engine::Tick();

        
        const auto& WindowSize = window->GetSize();
        glViewport(0, 0, WindowSize.x, WindowSize.y);

        RenderCommand::SetClearColor({ 0.1, 0.1 ,0.1, 1.0 });
        RenderCommand::Clear();
        
        Nuake::Engine::EndDraw();
    }
}
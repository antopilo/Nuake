#include <Engine.h>
#include "src/Core/Maths.h"
#include <src/Scene/Entities/Entity.h>

#include <src/Scene/Scene.h>
#include <src/Scene/Components/QuakeMap.h>
#include <src/Vendors/imgui/imgui.h>
#include <src/Vendors/imgui/ImGuizmo.h>
#include <src/Scene/Components/Components.h>
#include <src/Core/Physics/PhysicsManager.h>
#include "src/Scene/Components/BoxCollider.h"
#include "src/EditorInterface.h"
#include "src/Core/Input.h"
#include <GLFW/glfw3.h>
#include <src/Vendors/glm/trigonometric.hpp>
#include <src/Scripting/ScriptingEngine.h>
#include <src/Resource/FGD/FGDFile.h>
#include <src/Rendering/Shaders/ShaderManager.h>
#include <src/Rendering/Renderer.h>
#include "src/UI/UserInterface.h"
#include "src/NewEditor.h"
#include <src/Scene/Components/BSPBrushComponent.h>

void OpenProject()
{
    // Parse the project and load it.
    std::string projectPath = Nuake::FileDialog::OpenFile("Project file|*.project;");

    Nuake::FileSystem::SetRootDirectory(projectPath + "/../");
    Ref<Nuake::Project> project = Nuake::Project::New();
    if (!project->Deserialize(Nuake::FileSystem::ReadFile(projectPath, true)))
    {
        Nuake::Logger::Log("Error loading project: " + projectPath, Nuake::CRITICAL);
        return;
    }

    project->FullPath = projectPath;
    Nuake::Engine::LoadProject(project);

    // Create new interface named test.
    //userInterface = UI::UserInterface::New("test");


    // Set current interface running.
    //Engine::GetCurrentScene()->AddInterface(userInterface);
}

int main()
{
    Nuake::Engine::Init();

    Nuake::EditorInterface editor;
    editor.BuildFonts();

    // Register Gizmo textures
    Ref<Nuake::Texture> lightTexture = Nuake::TextureManager::Get()->GetTexture("resources/Icons/Gizmo/Light.png");
    Ref<Nuake::Texture> camTexture = Nuake::TextureManager::Get()->GetTexture("resources/Icons/Gizmo/Camera.png");
    Ref<Nuake::Shader> GuizmoShader = Nuake::ShaderManager::GetShader("resources/Shaders/gizmo.shader");
    Ref<Nuake::Shader> ditherShader = Nuake::ShaderManager::GetShader("resources/Shaders/dither.shader");

    //Nuake::NewEditor newEditor = Nuake::NewEditor();
    
    // Register shaders
    while (!Nuake::Engine::GetCurrentWindow()->ShouldClose())
    {
        Nuake::RenderCommand::Clear();
        Nuake::Engine::Tick();

        Nuake::Engine::Draw();

        //newEditor.Update(0.f);

        
        
        Nuake::Vector2 WindowSize = Nuake::Engine::GetCurrentWindow()->GetSize();
        glViewport(0, 0, WindowSize.x, WindowSize.y);
        Nuake::Renderer2D::BeginDraw(WindowSize);

        //newEditor.Draw(WindowSize);

        Ref<Nuake::FrameBuffer> sceneFramebuffer = Nuake::Engine::GetCurrentWindow()->GetFrameBuffer();
        sceneFramebuffer->Bind();
        
        Ref<Nuake::Scene> currentScene = Nuake::Engine::GetCurrentScene();
        if (currentScene && !Nuake::Engine::IsPlayMode)
        {
            //GuizmoShader->Bind();
            //
            //glDisable(GL_CULL_FACE);
            //glDisable(GL_DEPTH_TEST);
            //auto camView = currentScene->m_Registry.view<Nuake::TransformComponent, Nuake::CameraComponent>();
            //for (auto e : camView) {
            //    auto [transformComponent, cam] = camView.get<Nuake::TransformComponent, Nuake::CameraComponent>(e);
            //
            //    GuizmoShader->SetUniformMat4f("model", transformComponent.GetGlobalTransform());
            //    GuizmoShader->SetUniformMat4f("view", currentScene->m_EditorCamera->GetTransform());
            //    GuizmoShader->SetUniformMat4f("projection", currentScene->m_EditorCamera->GetPerspective());
            //
            //    camTexture->Bind(2);
            //    GuizmoShader->SetUniform1i("gizmo_texture", 2);
            //
            //    Nuake::Renderer::DrawQuad(transformComponent.GetGlobalTransform());
            //}
            //
            //auto view = currentScene->m_Registry.view<Nuake::TransformComponent, Nuake::LightComponent>();
            //for (auto e : view) {
            //    auto [transformComponent, light] = view.get<Nuake::TransformComponent, Nuake::LightComponent>(e);
            //
            //    GuizmoShader->SetUniformMat4f("model", transformComponent.GetGlobalTransform());
            //    GuizmoShader->SetUniformMat4f("view", currentScene->m_EditorCamera->GetTransform());
            //    GuizmoShader->SetUniformMat4f("projection", currentScene->m_EditorCamera->GetPerspective());
            //
            //    lightTexture->Bind(2);
            //    GuizmoShader->SetUniform1i("gizmo_texture", 2);
            //
            //    Nuake::Renderer::DrawQuad(transformComponent.GetGlobalTransform());
            //}
            //
            //glEnable(GL_CULL_FACE);
            //
            //GuizmoShader->Unbind();
            //
            //ditherShader->Bind();
            //ditherShader->SetUniformMat4f("u_View", Nuake::Engine::GetCurrentScene()->m_EditorCamera->GetTransform());
            //ditherShader->SetUniformMat4f("u_Projection", Nuake::Engine::GetCurrentScene()->m_EditorCamera->GetPerspective());
            //ditherShader->SetUniform1f("u_Time", Nuake::Engine::GetTime());
            //ditherShader->SetUniform4f("u_Color", 252.0 / 255.0, 3.0 / 255.0, 65.0 / 255.0, 1.0);
            //
            //if (editor.m_IsEntitySelected && editor.m_SelectedEntity.HasComponent<Nuake::BSPBrushComponent>())
            //{
            //    for (auto& m : editor.m_SelectedEntity.GetComponent<Nuake::BSPBrushComponent>().Meshes)
            //        Nuake::Renderer::SubmitMesh(m, editor.m_SelectedEntity.GetComponent<Nuake::TransformComponent>().GetGlobalTransform());
            //
            //    Nuake::Renderer::Flush(ditherShader, true);
            //}
            glEnable(GL_DEPTH_TEST);
        }
        sceneFramebuffer->Unbind();

        editor.Draw();
        Nuake::Engine::EndDraw();
    }

    Nuake::Engine::Close();
}


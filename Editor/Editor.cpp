#include <Engine.h>
#include <src/Core/Maths.h>
#include "src/Core/Input.h"

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

std::string WindowTitle = "Nuake Engine";

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
}

std::vector<Nuake::LineVertex> vertices
{
    Nuake::LineVertex {{10000.f, 0.0f, 0.0f},  {1.f, 0.f, 0.f, 1.f}},
    Nuake::LineVertex {{0.0f,    0.0f, 0.0f }, {1.f, 0.f, 0.f, 1.f}},
    Nuake::LineVertex {{0.f,      0.f, 10000.f }, {0.f, 1.f, 0.f, 1.f}},
    Nuake::LineVertex {{0.0f,    0.0f, 0.0f }, {0.f, 1.f, 0.f, 1.f}},
    Nuake::LineVertex {{0.f,  10000.f, 0.0f }, {0.f, 0.f, 1.f, 1.f}},
    Nuake::LineVertex {{0.0f,    0.0f, 0.0f }, {0.f, 0.f, 1.f, 1.f}}
};

int main()
{
    Nuake::Engine::Init();

    Nuake::EditorInterface editor;
    editor.BuildFonts();

    glLineWidth(2.0f);

    Nuake::Shader* lineShader = Nuake::ShaderManager::GetShader("resources/Shaders/line.shader");
    
    lineShader->Bind();

    Nuake::VertexArray* lineVertexArray = new Nuake::VertexArray();
    lineVertexArray->Bind();
    Nuake::VertexBuffer* lineVertexBuffer = new Nuake::VertexBuffer(vertices.data(), vertices.size() * sizeof(Nuake::LineVertex));
    Nuake::VertexBufferLayout* vblayout = new Nuake::VertexBufferLayout();
    vblayout->Push<float>(3);
    vblayout->Push<float>(4);
    lineVertexArray->AddBuffer(*lineVertexBuffer, *vblayout);

    // Register Gizmo textures
    Ref<Nuake::Texture> lightTexture = Nuake::TextureManager::Get()->GetTexture("resources/Icons/Gizmo/Light.png");
    Ref<Nuake::Texture> camTexture = Nuake::TextureManager::Get()->GetTexture("resources/Icons/Gizmo/Camera.png");
       Nuake::Shader* GuizmoShader = Nuake::ShaderManager::GetShader("resources/Shaders/gizmo.shader");
       Nuake::Shader* ditherShader = Nuake::ShaderManager::GetShader("resources/Shaders/dither.shader");

    //Nuake::NewEditor newEditor = Nuake::NewEditor();
    Ref<Nuake::Window> window = Nuake::Engine::GetCurrentWindow();
    window->SetTitle("Nuake Editor");

    while (!window->ShouldClose())
    {
        Nuake::Engine::Tick();
        Nuake::Engine::Draw();

        Nuake::Vector2 WindowSize = window->GetSize();
        glViewport(0, 0, WindowSize.x, WindowSize.y);
        Nuake::Renderer2D::BeginDraw(WindowSize);

        //newEditor.Draw(WindowSize);

        Ref<Nuake::FrameBuffer> sceneFramebuffer = window->GetFrameBuffer();
        sceneFramebuffer->Bind();

        Ref<Nuake::Scene> currentScene = Nuake::Engine::GetCurrentScene();
        if (currentScene && !Nuake::Engine::IsPlayMode)
        {
            glDisable(GL_DEPTH_TEST);

            lineShader->Bind();
            lineShader->SetUniformMat4f("u_View", Nuake::Engine::GetCurrentScene()->m_EditorCamera->GetTransform());
            lineShader->SetUniformMat4f("u_Projection", Nuake::Engine::GetCurrentScene()->m_EditorCamera->GetPerspective());

            lineVertexArray->Bind();
            Nuake::RenderCommand::DrawLines(0, 6);

            glEnable(GL_DEPTH_TEST);

            GuizmoShader->Bind();
            
            glDisable(GL_CULL_FACE);
            glDisable(GL_DEPTH_TEST);
            auto camView = currentScene->m_Registry.view<Nuake::TransformComponent, Nuake::CameraComponent>();
            for (auto e : camView) {
                auto [transformComponent, cam] = camView.get<Nuake::TransformComponent, Nuake::CameraComponent>(e);
                
                Nuake::Vector3 cameraPos = currentScene->m_EditorCamera->GetTranslation();
                Nuake::Vector3 position = transformComponent.GlobalTranslation;
                Nuake::Vector3 look = normalize(cameraPos - position);
                Nuake::Vector3 right = glm::cross(currentScene->m_EditorCamera->GetUp(), look);
                Nuake::Vector3 up2 = currentScene->m_EditorCamera->GetUp();
                Nuake::Matrix4 transform = glm::identity<Nuake::Matrix4>();
                transform[0] = Nuake::Vector4(right, 0);
                transform[1] = Nuake::Vector4(up2, 0);
                transform[2] = Nuake::Vector4(look, 0);
                transform = glm::translate(transform, position);
                GuizmoShader->SetUniformMat4f("model", transform);
                GuizmoShader->SetUniformMat4f("view", currentScene->m_EditorCamera->GetTransform());
                GuizmoShader->SetUniformMat4f("projection", currentScene->m_EditorCamera->GetPerspective());
            
                camTexture->Bind(2);
                GuizmoShader->SetUniform1i("gizmo_texture", 2);
            
                Nuake::Renderer::DrawQuad(transformComponent.GetGlobalTransform());
            }
            
            auto view = currentScene->m_Registry.view<Nuake::TransformComponent, Nuake::LightComponent>();
            for (auto e : view) {
                auto [transformComponent, light] = view.get<Nuake::TransformComponent, Nuake::LightComponent>(e);
            
                GuizmoShader->SetUniformMat4f("model", transformComponent.GetGlobalTransform());
                GuizmoShader->SetUniformMat4f("view", currentScene->m_EditorCamera->GetTransform());
                GuizmoShader->SetUniformMat4f("projection", currentScene->m_EditorCamera->GetPerspective());
            
                lightTexture->Bind(2);
                GuizmoShader->SetUniform1i("gizmo_texture", 2);
            
                Nuake::Renderer::DrawQuad(transformComponent.GetGlobalTransform());
            }
            
            glEnable(GL_CULL_FACE);
            
            GuizmoShader->Unbind();
            
            ditherShader->Bind();
            ditherShader->SetUniformMat4f("u_View", Nuake::Engine::GetCurrentScene()->m_EditorCamera->GetTransform());
            ditherShader->SetUniformMat4f("u_Projection", Nuake::Engine::GetCurrentScene()->m_EditorCamera->GetPerspective());
            ditherShader->SetUniform1f("u_Time", Nuake::Engine::GetTime());
            ditherShader->SetUniform4f("u_Color", 252.0 / 255.0, 3.0 / 255.0, 65.0 / 255.0, 1.0);
            
            if (editor.Selection.Type == EditorSelectionType::Entity && editor.Selection.Entity.HasComponent<Nuake::BSPBrushComponent>())
            {
                for (auto& m : editor.Selection.Entity.GetComponent<Nuake::BSPBrushComponent>().Meshes)
                    Nuake::Renderer::SubmitMesh(m, editor.Selection.Entity.GetComponent<Nuake::TransformComponent>().GetGlobalTransform());
            
                Nuake::Renderer::Flush(ditherShader, true);
            }
            
            ditherShader->SetUniform1f("u_Time", Nuake::Engine::GetTime() / 10.0f);
            ditherShader->SetUniform4f("u_Color", 52.f / 255.f, 235.f / 255.f, 88.f / 255.f, 1);
            auto cubeCollidersView = currentScene->m_Registry.view<Nuake::TransformComponent, Nuake::BoxColliderComponent>();
            if (editor.Selection.Type == EditorSelectionType::Entity && editor.Selection.Entity.HasComponent<Nuake::BoxColliderComponent>())
            {
                auto transformComponent = editor.Selection.Entity.GetComponent<Nuake::TransformComponent>();
                auto colliderComponent = editor.Selection.Entity.GetComponent<Nuake::BoxColliderComponent>();
                Nuake::Matrix4 transform = transformComponent.GetGlobalTransform();
                transform = glm::scale(transform, colliderComponent.Size);
                Nuake::Renderer::SubmitCube(transform);

                Nuake::Renderer::Flush(ditherShader, true);
            }
            glEnable(GL_DEPTH_TEST);
        }
        sceneFramebuffer->Unbind();

        editor.Draw();

        Nuake::Engine::EndDraw();
    }

    Nuake::Engine::Close();
}
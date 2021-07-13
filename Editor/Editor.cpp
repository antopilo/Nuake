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


int main()
{
    Nuake::Engine::Init();

    Nuake::EditorInterface editor;
    editor.BuildFonts();

    // Register Gizmo textures
    Ref<Nuake::Texture> lightTexture = Nuake::TextureManager::Get()->GetTexture("resources/Icons/Gizmo/Light.png");
    Ref<Nuake::Texture> camTexture = Nuake::TextureManager::Get()->GetTexture("resources/Icons/Gizmo/Camera.png");
    Ref<Nuake::Shader> GuizmoShader = Nuake::ShaderManager::GetShader("resources/Shaders/gizmo.shader");

    while (!Nuake::Engine::GetCurrentWindow()->ShouldClose())
    {
        Nuake::Engine::Tick();
        Nuake::Engine::Draw();

        if (Nuake::Input::IsKeyPressed(GLFW_KEY_F8))
            Nuake::Engine::ExitPlayMode();

        Ref<Nuake::FrameBuffer> sceneFramebuffer = Nuake::Engine::GetCurrentWindow()->GetFrameBuffer();
        sceneFramebuffer->Bind();
        
        Ref<Nuake::Scene> currentScene = Nuake::Engine::GetCurrentScene();
        if (currentScene && !Nuake::Engine::IsPlayMode)
        {
            GuizmoShader->Bind();
        
            glDisable(GL_CULL_FACE);
            glDisable(GL_DEPTH_TEST);
            auto camView = currentScene->m_Registry.view<Nuake::TransformComponent, Nuake::CameraComponent>();
            for (auto e : camView) {
                auto [transformComponent, cam] = camView.get<Nuake::TransformComponent, Nuake::CameraComponent>(e);

                GuizmoShader->SetUniformMat4f("model", transformComponent.GetTransform());
                GuizmoShader->SetUniformMat4f("view", currentScene->m_EditorCamera->GetTransform());
                GuizmoShader->SetUniformMat4f("projection", currentScene->m_EditorCamera->GetPerspective());

                camTexture->Bind(2);
                GuizmoShader->SetUniform1i("gizmo_texture", 2);

                Nuake::Renderer::DrawQuad(transformComponent.GetTransform());
            }

            auto view = currentScene->m_Registry.view<Nuake::TransformComponent, Nuake::LightComponent>();
            for (auto e : view) {
                auto [transformComponent, light] = view.get<Nuake::TransformComponent, Nuake::LightComponent>(e);

                GuizmoShader->SetUniformMat4f("model", transformComponent.GetTransform());
                GuizmoShader->SetUniformMat4f("view", currentScene->m_EditorCamera->GetTransform());
                GuizmoShader->SetUniformMat4f("projection", currentScene->m_EditorCamera->GetPerspective());

                lightTexture->Bind(2);
                GuizmoShader->SetUniform1i("gizmo_texture", 2);
            
                Nuake::Renderer::DrawQuad(transformComponent.GetTransform());
            }
            glEnable(GL_CULL_FACE);
            glEnable(GL_DEPTH_TEST);
            GuizmoShader->Unbind();
        }
        
        sceneFramebuffer->Unbind();
        editor.Draw();
        Nuake::Engine::EndDraw();
    }

    Nuake::Engine::Close();
}


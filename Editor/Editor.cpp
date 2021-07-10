#include <Engine.h>
#include <src/Scene/Entities/Entity.h>

#include <src/Scene/Scene.h>
#include <src/Scene/Components/QuakeMap.h>
#include <src/Vendors/imgui/imgui.h>
#include <src/Vendors/imgui/ImGuizmo.h>
#include <src/Scene/Components/Components.h>
#include <src/Core/Physics/PhysicsManager.h>
#include "../Scene/Components/BoxCollider.h"
#include "src/EditorInterface.h"
#include "../Core/Input.h"
#include <GLFW/glfw3.h>
#include <src/Vendors/glm/trigonometric.hpp>
#include <src/Scripting/ScriptingEngine.h>
#include <src/Resource/FGD/FGDFile.h>
#include <src/Rendering/Shaders/ShaderManager.h>
#include <src/Rendering/Renderer.h>

int main()
{
    Engine::Init();

    EditorInterface editor;
    editor.BuildFonts();

    // Register Gizmo textures
    Ref<Texture> lightTexture = TextureManager::Get()->GetTexture("resources/Icons/Gizmo/Light.png");
    Ref<Texture> camTexture = TextureManager::Get()->GetTexture("resources/Icons/Gizmo/Camera.png");
    Ref<Shader> GuizmoShader = ShaderManager::GetShader("resources/Shaders/gizmo.shader");

    while (!Engine::GetCurrentWindow()->ShouldClose())
    {
        Engine::Tick();
        Engine::Draw();

        if (Input::IsKeyPressed(GLFW_KEY_F8))
            Engine::ExitPlayMode();

        Ref<FrameBuffer> sceneFramebuffer = Engine::GetCurrentWindow()->GetFrameBuffer();
        sceneFramebuffer->Bind();
        
        Ref<Scene> currentScene = Engine::GetCurrentScene();
        if (currentScene && !Engine::IsPlayMode)
        {
            GuizmoShader->Bind();
        
            glDisable(GL_CULL_FACE);
            glDisable(GL_DEPTH_TEST);
            auto camView = currentScene->m_Registry.view<TransformComponent, CameraComponent>();
            for (auto e : camView) {
                auto [transformComponent, cam] = camView.get<TransformComponent, CameraComponent>(e);

                GuizmoShader->SetUniformMat4f("model", transformComponent.GetTransform());
                GuizmoShader->SetUniformMat4f("view", currentScene->m_EditorCamera->GetTransform());
                GuizmoShader->SetUniformMat4f("projection", currentScene->m_EditorCamera->GetPerspective());

                camTexture->Bind(2);
                GuizmoShader->SetUniform1i("gizmo_texture", 2);

                Renderer::DrawQuad(transformComponent.GetTransform());
            }

            auto view = currentScene->m_Registry.view<TransformComponent, LightComponent>();
            for (auto e : view) {
                auto [transformComponent, light] = view.get<TransformComponent, LightComponent>(e);

                GuizmoShader->SetUniformMat4f("model", transformComponent.GetTransform());
                GuizmoShader->SetUniformMat4f("view", currentScene->m_EditorCamera->GetTransform());
                GuizmoShader->SetUniformMat4f("projection", currentScene->m_EditorCamera->GetPerspective());

                lightTexture->Bind(2);
                GuizmoShader->SetUniform1i("gizmo_texture", 2);
            
                Renderer::DrawQuad(transformComponent.GetTransform());
            }
            glEnable(GL_CULL_FACE);
            glEnable(GL_DEPTH_TEST);
            GuizmoShader->Unbind();
        }
        
        sceneFramebuffer->Unbind();
        editor.Draw();
        Engine::EndDraw();
    }

    Engine::Close();
}


#include "EditorLayer.h"

#include "Windows/EditorInterface.h"
#include "Misc/GizmoDrawer.h"

#include "src/Core/Input.h"

#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>

#include "glad/glad.h"

void EditorLayer::OnAttach()
{
    m_EditorInterface = new Nuake::EditorInterface(mCommandBuffer);
    m_GizmoDrawer = new GizmoDrawer(m_EditorInterface);
}

void EditorLayer::OnUpdate()
{
    using namespace Nuake;

    Nuake::Engine::Tick();
    Nuake::Engine::Draw();

    m_EditorInterface->Draw();
    m_EditorInterface->Update(Nuake::Engine::GetTimestep());
    /*
    auto sceneFramebuffer = GetApplication().GetWindow()->GetFrameBuffer();

    // Draw gizmos
    if (Ref<Nuake::Scene> currentScene = Nuake::Engine::GetCurrentScene(); currentScene)
    {
        sceneFramebuffer->Bind();
        {
            Ref<EditorCamera> camera;
            if (currentScene)
            {
                camera = currentScene->m_EditorCamera;
                //currentScene->m_SceneRenderer->GetGBuffer().GetTexture(GL_COLOR_ATTACHMENT3)->Unbind();
                //glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, currentScene->m_SceneRenderer->GetGBuffer().GetTexture(GL_COLOR_ATTACHMENT3)->GetID(), 0);
                glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, currentScene->m_SceneRenderer->GetGBuffer().GetTexture(GL_DEPTH_ATTACHMENT)->GetID(), 0);
            }

            if (currentScene && !Nuake::Engine::IsPlayMode())
            {
                glEnable(GL_LINE_SMOOTH);

                if (m_EditorInterface->ShouldDrawAxis())
                {
                    m_GizmoDrawer->DrawAxis(currentScene, false);
                }

                if (m_EditorInterface->ShouldDrawNavMesh())
                {
                    m_GizmoDrawer->DrawNavMesh(currentScene, true);
                }

                if (m_EditorInterface->ShouldDrawGizmos())
                {
                    m_GizmoDrawer->DrawGizmos(currentScene, false);
                }

                if (m_EditorInterface->ShouldDrawShapes())
                {
                    m_GizmoDrawer->DrawShapes(currentScene, false);
                }
            }
        }

        sceneFramebuffer->Unbind();
    }

    */


    Nuake::Engine::EndDraw();
    Nuake::Input::Update();
}

void EditorLayer::OnDetach()
{
	delete m_EditorInterface;
}

void EditorLayer::OnWindowFocused()
{
    m_EditorInterface->OnWindowFocused();
}

void EditorLayer::OnDragNDrop(const std::vector<std::string>& paths)
{
    m_EditorInterface->OnDragNDrop(paths);
}

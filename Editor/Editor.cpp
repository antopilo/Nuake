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

#include "src/Misc/GizmoDrawer.h"

const std::string WindowTitle = "Nuake Editor";

int main()
{
    Nuake::Engine::Init();
    Nuake::EditorInterface editor;
    editor.BuildFonts();

    Ref<Nuake::Window> window = Nuake::Engine::GetCurrentWindow();
    window->SetTitle(WindowTitle);

    using namespace Nuake;

    GizmoDrawer gizmoDrawer = GizmoDrawer();

    while (!window->ShouldClose())
    {
        Nuake::Engine::Tick();
        Nuake::Engine::Draw();

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

			if (currentScene && !Nuake::Engine::IsPlayMode)
			{
				gizmoDrawer.DrawGizmos(currentScene);
			}
        }
        sceneFramebuffer->Unbind();

        editor.Draw();

        // Swap buffers.
        Nuake::Engine::EndDraw();
    }

    Nuake::Engine::Close();
}
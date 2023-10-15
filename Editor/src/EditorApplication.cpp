#include "EditorApplication.h"

#include "Engine.h"
#include "Windows/EditorInterface.h"
#include "Misc/GizmoDrawer.h"
#include "EditorLayer.h"

#include <glad/glad.h>


void EditorApplication::OnInit()
{
    using namespace Nuake;
    
    Engine::Init();
    m_Window = Engine::GetCurrentWindow();
    m_Window->SetSize({ m_Specification.WindowWidth, m_Specification.WindowHeight });
    m_Window->SetTitle(m_Specification.Name);
    
    PushLayer(CreateScope<EditorLayer>());
}

void EditorApplication::OnShutdown()
{
    Nuake::Engine::Close();
}
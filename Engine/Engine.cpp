#include "Engine.h"
#include "src/Core/Physics/PhysicsManager.h"
#include <GLFW/glfw3.h>
#include "src/Core/FileSystem.h"

float Engine::m_LastFrameTime = 0.0f;
Ref<Window> Engine::CurrentWindow;
bool Engine::IsPlayMode = false;

#include "../Rendering/Renderer.h"
void Engine::Init()
{
	
	PhysicsManager::Get()->Init();
	FileSystem::Scan();

	CurrentWindow = std::make_shared<Window>();

	
}

void Engine::Tick()
{
	float time = (float)glfwGetTime();
	Timestep timestep = time - m_LastFrameTime;
	m_LastFrameTime = time;

	if (Engine::IsPlayMode)
		GetCurrentScene()->Update(timestep);
	else
		GetCurrentScene()->EditorUpdate(timestep);
}

void Engine::EnterPlayMode()
{
	if (!IsPlayMode)
	{
		GetCurrentScene()->OnInit();
	}
	IsPlayMode = true;
}

void Engine::ExitPlayMode()
{
	if (IsPlayMode)
		GetCurrentScene()->OnExit();
	IsPlayMode = false;
}

void Engine::Draw() 
{
	Window::Get()->Draw();
}

void Engine::EndDraw()
{
	Window::Get()->EndDraw();
}

void Engine::Run()
{

}

void Engine::Close()
{
	glfwTerminate();
}

Ref<Scene> Engine::GetCurrentScene()
{
	return CurrentWindow->GetScene();
}

int Engine::HelloWorld()
{
	return 1;
}

Ref<Window> Engine::GetCurrentWindow()
{
	return CurrentWindow;
}
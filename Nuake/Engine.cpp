#include "Engine.h"
#include "src/Core/Physics/PhysicsManager.h"
#include <GLFW/glfw3.h>
#include "src/Core/FileSystem.h"

float Engine::m_LastFrameTime = 0.0f;
Ref<Window> Engine::CurrentWindow;
bool Engine::IsPlayMode = false;

Ref<Project> Engine::CurrentProject;

#include "../Rendering/Renderer.h"
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>
void Engine::Init()
{
	PhysicsManager::Get()->Init();
	FileSystem::Scan();

	CurrentWindow = std::make_shared<Window>();
}

void Engine::Tick()
{
	if (!CurrentWindow->GetScene())
		return;

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
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();

	ImGui::NewFrame();

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

bool Engine::LoadScene(Ref<Scene> scene)
{
	CurrentWindow->SetScene(scene);
	return true;
}

Ref<Project> Engine::GetProject()
{
	return CurrentProject;
}

bool Engine::LoadProject(Ref<Project> project)
{
	CurrentProject = project;
	
	Ref<Scene> newScene = CreateRef<Scene>();
	Engine::LoadScene(newScene);

	return true;
}

int Engine::HelloWorld()
{
	return 1;
}

Ref<Window> Engine::GetCurrentWindow()
{
	return CurrentWindow;
}
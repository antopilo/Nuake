#include "Engine.h"
#include "src/Core/Physics/PhysicsManager.h"
#include <GLFW/glfw3.h>
#include "src/Core/FileSystem.h"
#include "src/Scripting/ScriptingEngine.h"

#include "../Rendering/Renderer.h"
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>


float Engine::m_LastFrameTime = 0.0f;
Ref<Window> Engine::CurrentWindow;
bool Engine::IsPlayMode = false;

Ref<Project> Engine::CurrentProject;

void Engine::Init()
{
	Logger::Log("Engine initialization...");

	PhysicsManager::Get()->Init();
	Logger::Log("Physics initialized");


	ScriptingEngine::Init();
	Logger::Log("Scripting engine initialized");

	CurrentWindow = std::make_shared<Window>();
	Logger::Log("Window initialized");

	Logger::Log("Engine initialized succesfully...");
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
	ScriptingEngine::Close();
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
	Engine::LoadScene(CurrentProject->DefaultScene);
	FileSystem::SetRootDirectory(project->FullPath + "/../");
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
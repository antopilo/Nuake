#include "Engine.h"
#include "src/Core/Physics/PhysicsManager.h"
#include <GLFW/glfw3.h>

#include "../Core/Input.h"
#include "src/Core/FileSystem.h"
#include "src/Scripting/ScriptingEngine.h"

#include "../Rendering/Renderer.h"
#include "../Rendering/Renderer2D.h"

#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>

Ref<Project> Engine::CurrentProject;
Ref<Window> Engine::CurrentWindow;
float Engine::m_LastFrameTime = 0.0f;
float Engine::m_FixedUpdateRate = 1.0 / 60.0f;
float Engine::m_FixedUpdateDifference = 0.f;
bool Engine::IsPlayMode = false;

void Engine::Init()
{
	Logger::Log("Engine initialization...");

	PhysicsManager::Get()->Init();
	Logger::Log("Physics initialized");


	CurrentWindow = Window::Get();
	Logger::Log("Window initialized");

	Renderer2D::Init();
	Logger::Log("2D renderer initialized");

	Logger::Log("Engine initialized succesfully!");
}

void Engine::Tick()
{
	// Dont update if no scene is loaded.
	if (!CurrentWindow->GetScene())
		return;

	// Calculate delta time
	float time = (float)glfwGetTime();
	Timestep timestep = time - m_LastFrameTime;
	m_LastFrameTime = time;

	// Play mode update vs editor update.
	if (Engine::IsPlayMode) {
		CurrentWindow->Update(timestep);
		m_FixedUpdateDifference += timestep;
		// Fixed update
		if (m_FixedUpdateDifference >= m_FixedUpdateRate)
		{
			// call update here.
			CurrentWindow->FixedUpdate(m_FixedUpdateRate);
			m_FixedUpdateDifference = 0.f;
		}
	}
	else
		GetCurrentScene()->EditorUpdate(timestep);

	


	Input::Update();
}

void Engine::EnterPlayMode()
{
	// Dont trigger init if already in player mode.
	if (!IsPlayMode)
		GetCurrentScene()->OnInit();

	IsPlayMode = true;
}

void Engine::ExitPlayMode()
{
	// Dont trigger exit if already not in play mode.
	if (IsPlayMode)
		GetCurrentScene()->OnExit();

	IsPlayMode = false;
}

void Engine::Draw() 
{
	// Start imgui frame
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	// Draw scene
	Window::Get()->Draw();
}

void Engine::EndDraw()
{
	// Swap buffer and draw imgui
	Window::Get()->EndDraw();
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
	return CurrentWindow->SetScene(scene);
}

Ref<Project> Engine::GetProject()
{
	return CurrentProject;
}

bool Engine::LoadProject(Ref<Project> project)
{
	CurrentProject = project;

	if (!Engine::LoadScene(CurrentProject->DefaultScene))
		return false;

	FileSystem::SetRootDirectory(project->FullPath + "/../");
	return true;
}

Ref<Window> Engine::GetCurrentWindow()
{
	return CurrentWindow;
}
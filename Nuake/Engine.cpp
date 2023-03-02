#include "Engine.h"
#include "src/Core/Physics/PhysicsManager.h"
#include <GLFW/glfw3.h>

#include "src/Core/Input.h"
#include "src/Core/FileSystem.h"
#include "src/Scripting/ScriptingEngine.h"

#include "src/Rendering/Renderer.h"
#include "src/Rendering/Renderer2D.h"

#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>

namespace Nuake
{
	Ref<Project> Engine::CurrentProject;
	Ref<Window> Engine::CurrentWindow;

	float Engine::m_LastFrameTime = 0.0f;
	float Engine::m_FixedUpdateRate = 1.0f / 90.0f;
	float Engine::m_FixedUpdateDifference = 0.f;
	float Engine::m_Time = 0.f;

	bool Engine::IsPlayMode = false;

	void Engine::Init()
	{
		Logger::Log("Engine initialization...");

		PhysicsManager::Get()->Init();
		Logger::Log("Physics initialized");

		// Creates the window
		CurrentWindow = Window::Get();
		Logger::Log("Window initialized");

		Renderer2D::Init();
		Logger::Log("2D renderer initialized");

		Logger::Log("Engine initialized succesfully!");
	}

	void Engine::Tick()
	{
		m_Time = (float)glfwGetTime();
		Timestep timestep = m_Time - m_LastFrameTime;
		m_LastFrameTime = m_Time;

		// Dont update if no scene is loaded.
		if (CurrentWindow->GetScene()) 
		{
			CurrentWindow->Update(timestep);

			// Play mode update all the entities, Editor does not.
			if (Engine::IsPlayMode) {
				m_FixedUpdateDifference += timestep;

				// Fixed update
				if (m_FixedUpdateDifference >= m_FixedUpdateRate) {
					CurrentWindow->FixedUpdate(m_FixedUpdateDifference);

					m_FixedUpdateDifference = 0.0f;
				}
			}
			else
				GetCurrentScene()->EditorUpdate(timestep);
		}

		Input::Update();
	}

	void Engine::EnterPlayMode()
	{
		// Dont trigger init if already in player mode.
		if (IsPlayMode)
			return;

		if (GetCurrentScene()->OnInit())
			IsPlayMode = true;
		else
			GetCurrentScene()->OnExit();
	}

	void Engine::ExitPlayMode()
	{
		// Dont trigger exit if already not in play mode.
		if (IsPlayMode) 
			GetCurrentScene()->OnExit();

		Input::ShowMouse();
		IsPlayMode = false;
	}

	void Engine::Draw()
	{
		Nuake::RenderCommand::Clear();

		// Start imgui frame
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		// Draw scene
		Window::Get()->Draw();
	}

	void Engine::EndDraw()
	{
		Window::Get()->EndDraw();
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
}

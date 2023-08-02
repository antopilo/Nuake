#include "Engine.h"
#include "src/Core/Physics/PhysicsManager.h"
#include <GLFW/glfw3.h>

#include "src/Core/Input.h"
#include "src/Core/FileSystem.h"
#include "src/Scripting/ScriptingEngine.h"
#include "src/Audio/AudioManager.h"

#include "src/Rendering/Renderer.h"
#include "src/Rendering/Renderer2D.h"

#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>

namespace Nuake
{
	Ref<Project> Engine::s_CurrentProject;
	Ref<Window> Engine::s_CurrentWindow;

	bool Engine::s_IsPlayMode = false;

	float Engine::s_LastFrameTime = 0.0f;
	float Engine::s_FixedUpdateRate = 1.0f / 90.0f;
	float Engine::s_FixedUpdateDifference = 0.f;
	float Engine::s_Time = 0.f;
	Timestep Engine::s_TimeStep = 0.f;

	void Engine::Init()
	{
		AudioManager::Get().Initialize();
		PhysicsManager::Get().Init();

		// Creates the window
		s_CurrentWindow = Window::Get();

		Input::Init();
		Renderer2D::Init();
		Logger::Log("Engine initialized");
	}

	void Engine::Tick()
	{
		s_Time = (float)glfwGetTime();
		s_TimeStep = s_Time - s_LastFrameTime;
		s_LastFrameTime = s_Time;

		//s_TimeStep = std::min((float)s_TimeStep, 0f);

		// Dont update if no scene is loaded.
		if (s_CurrentWindow->GetScene()) 
		{
			s_CurrentWindow->Update(s_TimeStep);

			// Play mode update all the entities, Editor does not.
			if (!Engine::IsPlayMode())
			{
				GetCurrentScene()->EditorUpdate(s_TimeStep);
			}

			s_FixedUpdateDifference += s_TimeStep;

			// Fixed update
			while (s_FixedUpdateDifference >= s_FixedUpdateRate) 
			{
				s_CurrentWindow->FixedUpdate(s_FixedUpdateDifference);

				s_FixedUpdateDifference -= s_FixedUpdateRate;
			}
		}

		Input::Update();
	}

	void Engine::EnterPlayMode()
	{
		s_LastFrameTime = (float)glfwGetTime();; // Reset timestep timer.

		// Dont trigger init if already in player mode.
		if (IsPlayMode())
		{
			Logger::Log("Cannot enter play mode if is already in play mode.", "engine", WARNING);
			return;
		}

		if (GetCurrentScene()->OnInit())
		{
			s_IsPlayMode = true;
		}
		else
		{
			Logger::Log("Cannot enter play mode. Scene OnInit failed", "engine", CRITICAL);
			GetCurrentScene()->OnExit();
		}
	}

	void Engine::ExitPlayMode()
	{
		// Dont trigger exit if already not in play mode.
		if (IsPlayMode())
		{
			GetCurrentScene()->OnExit();
			Input::ShowMouse();
			s_IsPlayMode = false;
		}
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
		return s_CurrentWindow->GetScene();
	}

	bool Engine::LoadScene(Ref<Scene> scene)
	{
		return s_CurrentWindow->SetScene(scene);
	}

	Ref<Project> Engine::GetProject()
	{
		return s_CurrentProject;
	}

	bool Engine::LoadProject(Ref<Project> project)
	{
		s_CurrentProject = project;

		if (!Engine::LoadScene(s_CurrentProject->DefaultScene))
		{
			return false;
		}

		FileSystem::SetRootDirectory(FileSystem::GetParentPath(project->FullPath));
		return true;
	}

	Ref<Window> Engine::GetCurrentWindow()
	{
		return s_CurrentWindow;
	}
}

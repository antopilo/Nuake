#include "Engine.h"
#include "src/Physics/PhysicsManager.h"
#include <GLFW/glfw3.h>

#include "src/AI/NavManager.h"
#include "src/Audio/AudioManager.h"
#include "src/Core/FileSystem.h"
#include "src/Core/Input.h"
#include "src/Rendering/Renderer.h"
#include "src/Rendering/Renderer2D.h"
#include "src/Scripting/ScriptingEngine.h"
#include "src/Scripting/ScriptingEngineNet.h"
#include "src/Threading/JobSystem.h"
#include "src/Modules/Modules.h"

#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>
#include <Tracy.hpp>

namespace Nuake
{

	Ref<Project> Engine::s_CurrentProject;
	Ref<Window> Engine::s_CurrentWindow;

	GameState Engine::s_GameState = GameState::Stopped;

	float Engine::s_LastFrameTime = 0.0f;
	float Engine::s_FixedUpdateRate = 1.0f / 90.0f;
	float Engine::s_FixedUpdateDifference = 0.f;
	float Engine::s_Time = 0.f;
	Timestep Engine::s_TimeStep = 0.f;
	float Engine::s_TimeScale = 1.0f;

	void Engine::Init()
	{
		AudioManager::Get().Initialize();
		PhysicsManager::Get().Init();
		NavManager::Get().Initialize();

		// Creates the window
		s_CurrentWindow = Window::Get();

		Input::Init();
		Renderer2D::Init();
		Logger::Log("Engine initialized");

		Modules::StartupModules();
	}

	void Engine::Tick()
	{
		ZoneScoped;

		JobSystem::Get().Update();


		s_Time = static_cast<float>(glfwGetTime());
		s_TimeStep = s_Time - s_LastFrameTime;
		s_LastFrameTime = s_Time;

		// Dont update if no scene is loaded.
		if (s_CurrentWindow->GetScene())
		{
			float scaledTimeStep = s_TimeStep * s_TimeScale;
			s_CurrentWindow->Update(scaledTimeStep);

			// Play mode update all the entities, Editor does not.
			if (!Engine::IsPlayMode())
			{
				GetCurrentScene()->EditorUpdate(scaledTimeStep);
			}

			s_FixedUpdateDifference += s_TimeStep;

			// Fixed update
			while (s_FixedUpdateDifference >= s_FixedUpdateRate)
			{
				s_CurrentWindow->FixedUpdate(s_FixedUpdateRate * s_TimeScale);

				s_FixedUpdateDifference -= s_FixedUpdateRate;
			}

			Input::Update();
			AudioManager::Get().AudioUpdate();
		}
	}

	void Engine::EnterPlayMode()
	{
		s_LastFrameTime = (float)glfwGetTime(); // Reset timestep timer.

		// Dont trigger init if already in player mode.
		if (GetGameState() == GameState::Playing)
		{
			Logger::Log("Cannot enter play mode if is already in play mode.", "engine", WARNING);
			return;
		}

		PhysicsManager::Get().ReInit();

		if (GetCurrentScene()->OnInit())
		{
			SetGameState(GameState::Playing);
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
		if (s_GameState != GameState::Stopped)
		{
			GetCurrentScene()->OnExit();
			Input::ShowMouse();
			s_GameState = GameState::Stopped;
		}
	}

	void Engine::Draw()
	{
		ZoneScoped;

		RenderCommand::Clear();

		// Start imgui frame
		{
			ZoneScopedN("ImGui New Frame");

			ImGui_ImplOpenGL3_NewFrame();
			ImGui_ImplGlfw_NewFrame();
			ImGui::NewFrame();
		}

		// Draw scene
		Window::Get()->Draw();
	}

	void Engine::EndDraw()
	{
		ZoneScoped;
		Window::Get()->EndDraw();
	}

	void Engine::Close()
	{
		glfwTerminate();
	}

	Ref<Scene> Engine::GetCurrentScene()
	{
		if (s_CurrentWindow)
		{
			return s_CurrentWindow->GetScene();
		}

		return nullptr;
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
		ScriptingEngineNet::Get().Initialize();
		ScriptingEngineNet::Get().LoadProjectAssembly(project);

		return true;
	}

	Ref<Window> Engine::GetCurrentWindow()
	{
		return s_CurrentWindow;
	}
}

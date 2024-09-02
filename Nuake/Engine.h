#pragma once
#include "src/Core/Core.h"
#include "src/Window.h"
#include "src/Scene/Scene.h"
#include "src/Resource/Project.h"
#include "src/Core/Logger.h"

/* TODOS:

*/

// Welcome to the Nuake source code.
namespace Nuake
{
	enum GameState
	{
		Playing,
		Paused,
		Stopped
	};

	class Engine 
	{
	private:
		static Ref<Window> s_CurrentWindow;
		static Ref<Project> s_CurrentProject;
		static Ref<Scene> s_CurrentScene;

		static GameState s_GameState;

		static float s_LastFrameTime;
		static float s_FixedUpdateRate;
		static float s_FixedUpdateDifference;
		static float s_Time;
		static Timestep s_TimeStep;
		static float s_TimeScale;
	public:
		static void Init();  // Initialize the engine.
		static void Tick();  // Updates everything, called every frame.
		static void Close(); // Shutdown the engine.

		static void EnterPlayMode(); // Start the game
		static void ExitPlayMode(); // Stops the game

		// Custom drawing should happen in between these two
		static void Draw();    // Start new frame
		static void EndDraw(); // Swap buffer

		static void SetGameState(GameState gameState) { s_GameState = gameState; }
		static GameState GetGameState() { return s_GameState; }
		static bool IsPlayMode() { return s_GameState == GameState::Playing; }

		static inline float GetTime() { return s_Time; }
		static inline Timestep GetTimestep() { return s_TimeStep; } 
		static inline void SetPhysicsStep(int amount) { s_FixedUpdateRate = 1.0f / static_cast<float>(amount); }
		static inline float GetFixedTimeStep() { return s_FixedUpdateRate; }
		static inline void SetTimeScale(float timeScale) { s_TimeScale = timeScale; }
		static inline float GetTimeScale() { return s_TimeScale; }

		static Ref<Window> GetCurrentWindow();

		static bool LoadScene(Ref<Scene> scene);
		static Ref<Scene> GetCurrentScene();

		static bool LoadProject(Ref<Project> project);
		static Ref<Project> GetProject();

	};
}


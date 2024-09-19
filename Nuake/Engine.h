#pragma once

#include "src/Core/Core.h"
#include "src/Core/Logger.h"
#include "src/Window.h"

// Welcome to the Nuake source code.
namespace Nuake
{
	class Project;
	class Scene;
	class EngineSubsystem;
	class EngineSubsystemScriptable;

	enum GameState
	{
		Loading,
		Playing,
		Paused,
		Stopped
	};

	class Engine 
	{
	public:
		static void Init();  // Initialize the engine.
		static void Tick();  // Updates everything, called every frame.
		static void Close(); // Shutdown the engine.

		static void EnterPlayMode(); // Start the game
		static void ExitPlayMode(); // Stops the game

		// Custom drawing should happen in between these two
		static void Draw();    // Start new frame
		static void EndDraw(); // Swap buffer

		static void SetGameState(GameState state) { gameState = state; }
		static GameState GetGameState() { return gameState; }
		static bool IsPlayMode() { return gameState == GameState::Playing; }

		static inline float GetTime() { return time; }
		static inline Timestep GetTimestep() { return timeStep; } 
		static inline void SetPhysicsStep(int amount) { fixedUpdateRate = 1.0f / static_cast<float>(amount); }
		static inline float GetFixedTimeStep() { return fixedUpdateRate; }
		static inline void SetTimeScale(float timeScale) { timeScale = timeScale; }
		static inline float GetTimeScale() { return timeScale; }

		static Ref<Window> GetCurrentWindow();

		static bool SetCurrentScene(Ref<Scene> scene);
		static bool QueueSceneSwitch(const std::string& scene);
		static Ref<Scene> GetCurrentScene();

		static bool LoadProject(Ref<Project> project);
		static Ref<Project> GetProject();

		static Ref<EngineSubsystemScriptable> GetScriptedSubsystem(const std::string& subsystemName);
		static Ref<EngineSubsystemScriptable> GetScriptedSubsystem(const int subsystemId);

	protected:
		static void OnWindowSetScene(Ref<Scene> oldScene, Ref<Scene> newScene);
		static void InitializeCoreSubsystems();
		static void OnScriptingEngineGameAssemblyLoaded();

		static void OnScenePreInitialize(Ref<Scene> scene);
		static void OnScenePostInitialize(Ref<Scene> scene);

	private:
		static Ref<Window> currentWindow;
		static Ref<Project> currentProject;
		static Ref<Scene> currentScene;
		static std::string queuedScene;

		static inline std::vector<Ref<EngineSubsystem>> subsystems;
		static inline std::unordered_map<std::string, Ref<EngineSubsystemScriptable>> scriptedSubsystemMap;

		static GameState gameState;

		static float lastFrameTime;
		static float fixedUpdateRate;
		static float fixedUpdateDifference;
		static float time;
		static Timestep timeStep;
		static float timeScale;

	};
}


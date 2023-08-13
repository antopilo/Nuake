#pragma once
#include "src/Core/Core.h"
#include "src/Window.h"
#include "src/Scene/Scene.h"
#include "src/Resource/Project.h"
#include "src/Core/Logger.h"

/* TODOS:
[ ] - Parse the bones
[ ] - Create SceneStructure Entities(with bone component)
[ ] - Create SkinnedMesh resource(?)
*/

// Welcome to the Nuake source code.
namespace Nuake
{
	class Engine 
	{
	private:
		static Ref<Window> s_CurrentWindow;
		static Ref<Project> s_CurrentProject;
		static Ref<Scene> s_CurrentScene;

		static bool s_IsPlayMode; 

		static float s_LastFrameTime;
		static float s_FixedUpdateRate;
		static float s_FixedUpdateDifference;
		static float s_Time;
		static Timestep s_TimeStep;

	public:
		static void Init();  // Initialize the engine.
		static void Tick();  // Updates everything, called everyframe.
		static void Close(); // Shutdown the engine.

		static void EnterPlayMode(); // Start the game
		static void ExitPlayMode(); // Stops the game

		// Custom drawing should happen in between these two
		static void Draw();    // Start new frame
		static void EndDraw(); // Swap buffer

		static bool IsPlayMode() { return s_IsPlayMode; }

		static inline float GetTime() { return s_Time; }
		static inline Timestep GetTimestep() { return s_TimeStep; } 

		static Ref<Window> GetCurrentWindow();

		static bool LoadScene(Ref<Scene> scene);
		static Ref<Scene> GetCurrentScene();

		static bool LoadProject(Ref<Project> project);
		static Ref<Project> GetProject();
	};
}


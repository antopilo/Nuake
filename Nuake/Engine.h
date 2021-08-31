#pragma once
#include "src/Core/Core.h"
#include "src/Window.h"
#include "src/Scene/Scene.h"
#include "src/Resource/Project.h"
#include "src/Core/Logger.h"

/* TODOS:
* 
* Engine:
*	Scripting API for editing UI maybe
*	Launch game standalone
*	Fix physics system
*/

// Welcome to the Nuake source code.
namespace Nuake
{
	class Engine {
	private:
		static Ref<Window> CurrentWindow;
		static Ref<Project> CurrentProject;
		static Ref<Scene> CurrentScene;

		static float m_LastFrameTime;
		static float m_FixedUpdateRate;
		static float m_FixedUpdateDifference;
		static float m_Time;
	public:
		static bool IsPlayMode; // Is currently in runtime..

		static void Init();  // Initialize the engine.
		static void Tick();  // Updates everything, called everyframe.
		static void Close(); // Shutdown the engine.

		static void EnterPlayMode(); // Start the game
		static void ExitPlayMode(); // Stops the game

		// Custom drawing should happen in between these two
		static void Draw();    // Start new frame
		static void EndDraw(); // Swap buffer
		static inline float GetTime() { return m_Time; }
		static Ref<Window> GetCurrentWindow();

		static bool LoadScene(Ref<Scene> scene);
		static Ref<Scene> GetCurrentScene();

		static bool LoadProject(Ref<Project> project);
		static Ref<Project> GetProject();
	};
}


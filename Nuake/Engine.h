#pragma once

#include "src/Core/Core.h"
#include "src/Window.h"
#include "src/Scene/Scene.h"
#include "src/Resource/Project.h"
#include "src/Scripting/ScriptLoader.h"
#include "src/Core/Logger.h"

/* TODOS:
* 
* Engine:
*	Trigger zones from trenchbroom
*	Scripting API vectors operation
*	Scripting API for all component
*	Scripting API for editing UI maybe
*	Launch game standalone
*	Fix physics system
*	trenchbroom physics element
*	
* Editor:
*   File browser refact
*   
*/

class Engine {
private:
	static float m_LastFrameTime;
	static Ref<Window> CurrentWindow;
	static Ref<Project> CurrentProject;
	static Ref<Scene> CurrentScene;

public:
	static bool IsPlayMode; // True if is playing a scene

	static void Init();  // Initialize the systems
	static void Tick();  // Update every system, called once per frame.
	static void Close(); // Close the systems
	 
	static void EnterPlayMode(); // Start the game
	static void ExitPlayMode(); // Stops the game

	// Custom drawing should happen in between these two
	static void Draw();    // Start new frame
	static void EndDraw(); // Swap buffer

	static Ref<Window> GetCurrentWindow();

	// Load a specific scene
	static bool LoadScene(Ref<Scene> scene);
	static Ref<Scene> GetCurrentScene();

	// Loads a project and the default scene.
	static bool LoadProject(Ref<Project> project);
	static Ref<Project> GetProject();
};

#pragma once
#include "src/Core/Core.h"
#include "src/Window.h"
#include "src/Scene/Scene.h"
#include "src/Resource/Project.h"

class Engine {
private:
	static float m_LastFrameTime;
	static Ref<Window> CurrentWindow;
	static Ref<Project> CurrentProject;

public:
	static bool IsPlayMode;
	static void Init();
	static void Run();
	static void Tick();

	static void EnterPlayMode();
	static void ExitPlayMode();

	static void Draw();
	static void EndDraw();
	static void Close();

	static int HelloWorld();
	static Ref<Window> GetCurrentWindow();
	static Ref<Scene> GetCurrentScene();
	static bool LoadScene(Ref<Scene> scene);

	static Ref<Project> GetProject();
	static bool LoadProject(Ref<Project> project);
};
#pragma once
#include "src/Core/Core.h"
#include "src/Window.h"
#include "src/Scene/Scene.h"


class __declspec(dllexport) Engine {
private:
	static float m_LastFrameTime;
	static Ref<Window> CurrentWindow;


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
};
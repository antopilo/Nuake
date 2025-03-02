#pragma once
#include "Application.h"

bool g_ApplicationRunning = true;

namespace Nuake {

	int Main(int argc, char** argv)
	{
		while (g_ApplicationRunning)
		{
			Application* app = CreateApplication(argc, argv);
			app->OnInit();
			app->Run();
			app->Exit();
			delete app;

			g_ApplicationRunning = false;
		}

		return 0;
	}
}

#ifdef NK_WIN
#ifdef NK_DIST
#include "windows.h"
int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hInstPrev, LPSTR cdmline, int cmdshow)
{
	return ApplicationMain(__argc, __argv);
}
#else 
#define REGULAR_MAIN
#endif
#else
#define REGULAR_MAIN
#endif

#ifdef REGULAR_MAIN
int main(int argc, char** argv)
{
	return Nuake::Main(argc, argv);
}
#endif

#include "Commands.h"

#include <Engine.h>

namespace NuakeEditor
{
	bool SaveSceneCommand::Execute()
	{
		return true;
	}

	bool SaveProjectCommand::Execute()
	{
		Engine::GetProject()->Save();
		Engine::GetCurrentScene()->Save();

		return true;
	}

	bool SetGameState::Execute()
	{
		GameState gameState = Engine::GetGameState();
		if (gameState == GameState::Stopped && mGameState == GameState::Playing)
		{
			Engine::EnterPlayMode();
		}
		else if (mGameState == GameState::Stopped)
		{
			Engine::ExitPlayMode();
		}

		Engine::SetGameState(mGameState);

		return true;
	}
}
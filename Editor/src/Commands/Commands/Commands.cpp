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
}
#pragma once

#include "../ICommand.h"

#include <Nuake/Core/Core.h>
#include <Nuake/Scene/Scene.h>
#include <Nuake/Resource/Project.h>
#include <Engine.h>

namespace NuakeEditor {
	using namespace Nuake;
	
	class SaveSceneCommand : public ICommand
	{
	private:
		Ref<Scene> mScene;

	public:
		SaveSceneCommand(Ref<Scene> scene) : mScene(scene) {}

		bool Execute() override;
	};

	class SaveProjectCommand : public ICommand
	{
	private:
		Ref<Project> mProject;

	public:
		SaveProjectCommand(Ref<Project> project) : mProject(project) {}

		bool Execute() override;
	};

	// GameState
	class SetGameState : public ICommand
	{
	private:
		GameState mGameState;

	public:
		SetGameState(GameState gameState) :
			mGameState(gameState)
		{
		}

		bool Execute() override;
	};

	class CreateTrenchbroomGameConfig : public ICommand
	{
	private:
		Ref<Project> mProject;

	public:
		CreateTrenchbroomGameConfig(Ref<Project> project) :
			mProject(project)
		{
		}

		bool Execute() override;
	};

	class SetVSync : public ICommand
	{
	private:
		bool value;

	public:
		SetVSync(bool value) : value(value)
		{
		}

		bool Execute() override;
	};

}
#pragma once

#include "../ICommand.h"

#include <src/Core/Core.h>
#include <src/Scene/Scene.h>
#include <src/Resource/Project.h>


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

}
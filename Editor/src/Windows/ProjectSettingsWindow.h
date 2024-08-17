#pragma once
#include "src/Core/Core.h"
#include "src/Resource/Project.h"

class ProjectSettingsCategoryWindow
{
public:

	void Draw();
};

class ProjectSettingsWindow
{
private:
	std::vector<ProjectSettingsCategoryWindow> CategoryWindows;

public:
	ProjectSettingsWindow(Ref<Nuake::Project> project);
	~ProjectSettingsWindow();

	void Draw();
};
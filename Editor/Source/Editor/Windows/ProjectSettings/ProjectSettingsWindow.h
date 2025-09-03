#pragma once
#include "Nuake/Core/Core.h"
#include "Nuake/Resource/Project.h"

class ProjectSettingsCategoryWindow
{
public:
	std::string Name = "NO_NAME";

	virtual void Draw() = 0;
};

class ProjectSettingsCategoryWindowGeneral : public ProjectSettingsCategoryWindow
{
private:
	Ref<Nuake::Project> m_Project;
public:
	ProjectSettingsCategoryWindowGeneral(Ref<Nuake::Project> project);

	void Draw() override;
};

class ProjectSettingsCategoryWindowViewport : public ProjectSettingsCategoryWindow
{
private:
	Ref<Nuake::Project> m_Project;
public:
	ProjectSettingsCategoryWindowViewport(Ref<Nuake::Project> project);

	void Draw() override;
};

class ProjectSettingsCategoryWindowRendering : public ProjectSettingsCategoryWindow
{
private:
	Ref<Nuake::Project> m_Project;
public:
	ProjectSettingsCategoryWindowRendering(Ref<Nuake::Project> project);

	void Draw() override;
};

class ProjectSettingsCategoryScripting : public ProjectSettingsCategoryWindow
{
private:
	Ref<Nuake::Project> m_Project;
public:
	ProjectSettingsCategoryScripting(Ref<Nuake::Project> project);

	void Draw() override;
};

class ProjectSettingsCategoryPhysics : public ProjectSettingsCategoryWindow
{
private:
	Ref<Nuake::Project> m_Project;
public:
	ProjectSettingsCategoryPhysics(Ref<Nuake::Project> project);

	void Draw() override;
};

class ProjectSettingsCategoryAudio : public ProjectSettingsCategoryWindow
{
private:
	Ref<Nuake::Project> m_Project;
public:
	ProjectSettingsCategoryAudio(Ref<Nuake::Project> project);

	void Draw() override;
};

class ProjectSettingsModuleWindow : public ProjectSettingsCategoryWindow
{
private:
	std::string moduleName;

public:
	ProjectSettingsModuleWindow(const std::string& inModuleName);

	void Draw() override;
};

class ProjectSettingsWindow
{
private:
	std::vector<Ref<ProjectSettingsCategoryWindow>> m_CategoryWindows;
	uint32_t m_CurrentTab = 0;
	Ref<Nuake::Project> m_Project;
public:
	bool m_DisplayWindow = false;
	ProjectSettingsWindow();
	~ProjectSettingsWindow() = default;

	void Init(Ref<Nuake::Project> project);
	void Draw();
};
#pragma once
#include "src/Core/Core.h"
#include "src/Resource/Project.h"


class ProjectInterface
{
public:
	Ref<Project> m_CurrentProject;


	void DrawProjectSettings();

	void DrawCreatePointEntity();
	void DrawEntitySettings();


};
#pragma once
#include "src/Core/Core.h"
#include "src/Resource/Project.h"

namespace Nuake {
	class ProjectInterface
	{
	public:
		Ref<Project> m_CurrentProject;

		void DrawProjectSettings();
		void DrawCreatePointEntity();
		void DrawEntitySettings();
	};
}

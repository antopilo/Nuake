#pragma once
#include "Nuake/Core/Core.h"
#include "Nuake/Resource/Project.h"

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

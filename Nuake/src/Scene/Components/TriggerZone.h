#pragma once
#include "src/Core/Physics/GhostObject.h"
#include <vector>

namespace Nuake {
	class TriggerZone {
	public:
		Ref<GhostObject> m_GhostObject;
		std::string target = "";

		std::vector<Entity> Targets;
		bool Enabled = true;

		TriggerZone() 
		{
			Targets = std::vector<Entity>();
		}

		int GetOverLappingCount()
		{
			if (!Enabled) return 0;

			return m_GhostObject->OverlappingCount();
		}

		std::vector<Entity> GetTargets() {
			return Targets;
		}

		std::vector<Entity> GetOverlappingBodies()
		{
			if (!Enabled)
				return std::vector<Entity>();

			return m_GhostObject->GetOverlappingEntities();
		}
	};
}

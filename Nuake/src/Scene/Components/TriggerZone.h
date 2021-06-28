#pragma once
#include "src/Core//Physics/GhostObject.h"
#include <vector>
#
class TriggerZone {
public:
	Ref<GhostObject> GhostObject;
	std::string target = "";

	std::vector<Entity> Targets;
	bool Enabled = true;

	TriggerZone() {
		Targets = std::vector<Entity>();
	}



	int GetOverLappingCount() 
	{
		if (!Enabled) return 0;

		return GhostObject->OverlappingCount();
	}

	std::vector<Entity> GetTargets() {
		return Targets;
	}

	std::vector<Entity> GetOverlappingBodies()
	{
		if (!Enabled)
			return std::vector<Entity>();

		return GhostObject->GetOverlappingEntities();
	}
};
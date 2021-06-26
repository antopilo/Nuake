#pragma once
#include "src/Core//Physics/GhostObject.h"
#include <vector>

class TriggerZone {
public:
	Ref<GhostObject> GhostObject;

	bool Enabled = true;

	TriggerZone() {
		
	}

	int GetOverLappingCount() 
	{
		//if (!Enabled) return 0;

		return GhostObject->OverlappingCount();
	}

	std::vector<Entity> GetOverlappingBodies()
	{
		if (!Enabled)
			return std::vector<Entity>();

		return GhostObject->GetOverlappingEntities();
	}
};
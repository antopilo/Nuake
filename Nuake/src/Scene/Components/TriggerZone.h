#pragma once

#include <vector>

namespace Nuake 
{
	class TriggerZone 
	{
	public:
		std::string target = "";

		std::vector<Entity> Targets;
		bool Enabled = true;

		TriggerZone() 
		{
			Targets = std::vector<Entity>();
		}

		std::vector<Entity> GetTargets() 
		{
			return Targets;
		}
		
	};
}

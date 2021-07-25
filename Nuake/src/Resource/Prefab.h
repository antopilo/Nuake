#pragma once
#include <string>
#include <vector>
#include <src/Scene/Entities/Entity.h>

namespace Nuake {
	class Prefab
	{
	public:
		std::string Path;
		std::vector<Entity> Entities;
		
		Prefab() 
		{
			Path = "";
			Entities = std::vector<Entity>();
		}
		
		void AddEntity(Entity& entity)
		{
			Entities.push_back(entity);
		}
	};
}

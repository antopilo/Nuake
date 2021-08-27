#pragma once
#include <string>
#include <vector>
#include <src/Scene/Entities/Entity.h>
#include "src/Resource/Serializable.h"
namespace Nuake {
	class Prefab : ISerializable
	{
	public:
		std::string Path;
		std::vector<Entity> Entities;
		
		static Ref<Prefab> CreatePrefabFromEntity(Entity entity);

		Prefab() 
		{
			Path = "";
			Entities = std::vector<Entity>();
		}
		
		void AddEntity(Entity& entity)
		{
			Entities.push_back(entity);
		}

		void SaveAs(const std::string& path)
		{
			Path = path;

			FileSystem::BeginWriteFile(path);
			FileSystem::WriteLine(Serialize().dump(4));
			FileSystem::EndWriteFile();
		}

		json Serialize()
		{
			BEGIN_SERIALIZE();
			SERIALIZE_VAL(Path);
			std::vector<json> entities = std::vector<json>();
			for (Entity e : Entities)
				entities.push_back(e.Serialize());
			SERIALIZE_VAL_LBL("Entities", entities);
			END_SERIALIZE();
		}

		bool Deserialize(const std::string& str)
		{
			BEGIN_DESERIALIZE();
			Path = j["Path"];

			return true;
		}

	private:
		void EntityWalker(Entity entity);
	};
}

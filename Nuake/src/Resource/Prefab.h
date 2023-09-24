#pragma once
#include "src/Scene/Entities/Entity.h"
#include "src/Scene/Components/ParentComponent.h"
#include "src/Resource/Serializable.h"
#include "Engine.h"

#include <string>
#include <vector>
namespace Nuake {
	class Prefab : ISerializable
	{
	public:
		std::string Path;
		std::vector<Entity> Entities;
		Entity Root;

		static Ref<Prefab> CreatePrefabFromEntity(Entity entity);

		static Ref<Prefab> New(const std::string& path);

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

			FileSystem::BeginWriteFile(path, true);
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

			j["Root"] = Root.GetComponent<NameComponent>().ID;
			SERIALIZE_VAL_LBL("Entities", entities);

			// Manual correction to remove parent if we are prefabing a child. 
			// We want to get rid of parent link in the prefab itself.
			for (auto& e : j["Entities"])
			{
				if (e["NameComponent"]["ID"] == Root.GetComponent<NameComponent>().ID)
					e["ParentComponent"]["HasParent"] = false;
			}
				
			END_SERIALIZE();
		}

		bool Deserialize(const json& j)
		{
			if (j == "")
				return false;

			Path = j["Path"];
			if (j.contains("Entities"))
			{
				const auto& scene = Engine::GetCurrentScene();

				for (json e : j["Entities"])
				{
					Entity entity = { scene->m_Registry.create(), scene.get() };
					entity.Deserialize(e); // Id gets overriden by serialized id.

					auto& nameComponent = entity.GetComponent<NameComponent>();
					bool isRoot = false;
					if (nameComponent.ID == j["Root"])
					{
						isRoot = true; // We found the root entity of the prefab.
					}

					nameComponent.Name = scene->GetUniqueEntityName(nameComponent.Name);
					nameComponent.ID = OS::GetTime();

					if (isRoot)
					{
						Root = entity;
					}

					this->AddEntity(entity);
				}

				// Set reference to the parent entity to children
				for (auto& e : Entities)
				{
					auto parentC = e.GetComponent<ParentComponent>();
					if (!parentC.HasParent)
						continue;

					auto parent = Engine::GetCurrentScene()->GetEntityByID(parentC.ParentID);
					parent.AddChild(e);
				}
			}
			return true;
		}

	private:
		void EntityWalker(Entity entity);
	};
}

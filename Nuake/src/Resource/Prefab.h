#pragma once
#include "src/Scene/Entities/Entity.h"
#include "src/Scene/Components/ParentComponent.h"
#include "src/Resource/Serializable.h"
#include "Engine.h"

#include <string>
#include <vector>
#include <map>
#include <src/Scene/Components/SkinnedModelComponent.h>

namespace Nuake {

	class Prefab : ISerializable
	{
	public:
		std::string DisplayName;
		std::string Description;
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
			SERIALIZE_VAL(DisplayName);
			SERIALIZE_VAL(Description);

			auto entities = std::vector<json>();
			for (Entity e : Entities)
			{
				entities.push_back(e.Serialize());
			}

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

			if (j.contains("DisplayName"))
			{
				DisplayName = j["DisplayName"];
			}

			if (j.contains("Description"))
			{
				Description = j["Description"];
			}

			if (j.contains("Entities"))
			{
				const auto& scene = Engine::GetCurrentScene();

				std::map<uint32_t, uint32_t> newIdsLut;

				for (json e : j["Entities"])
				{
					Entity entity = { scene->m_Registry.create(), scene.get() };
					entity.Deserialize(e); // Id gets overriden by serialized id.

					auto& nameComponent = entity.GetComponent<NameComponent>();
					bool isRoot = false;
					if (nameComponent.ID == j["Root"])
					{
						isRoot = true; // We found the root entity of the prefab.
						auto& parentComponent = entity.GetComponent<ParentComponent>();
						parentComponent.HasParent = false;
						parentComponent.ParentID = 0;
					}

					uint32_t oldId = nameComponent.ID;
					uint32_t newId = OS::GetTime();
					nameComponent.Name = nameComponent.Name;
					nameComponent.ID = newId;

					newIdsLut[oldId] = newId;
					if (isRoot)
					{
						Root = entity;
					}

					AddEntity(entity);
				}

				// Set reference to the parent entity to children
				for (auto& e : Entities)
				{
					if (e.GetID() == Root.GetID())
					{
						continue;
					}

					auto& parentC = e.GetComponent<ParentComponent>();
					auto parent = Engine::GetCurrentScene()->GetEntityByID(newIdsLut[parentC.ParentID]);
					parent.AddChild(e);
				}

				// Since the bones point to an entity, and we are instancing a prefab, the new skeleton is gonna be pointing to the wrong
				// bones, we need to remap the skeleton to the new entities. We are simply reussing the same map we are using for the 
				// reparenting. Pretty neat.
				std::function<void(SkeletonNode&)> recursiveBoneRemapping = [&recursiveBoneRemapping, &newIdsLut](SkeletonNode& currentBone) 
				{
					for (SkeletonNode& bone : currentBone.Children)
					{
						bone.EntityHandle = newIdsLut[bone.EntityHandle];
						recursiveBoneRemapping(bone);
					}
				};

				// Do the remapping of the skeleton
				for (auto& e : Entities)
				{
					if (e.GetID() == Root.GetID() || !e.HasComponent<SkinnedModelComponent>())
					{
						continue;
					}

					auto& skinnedModelComponent = e.GetComponent<SkinnedModelComponent>();
					if (!skinnedModelComponent.ModelResource)
					{
						continue;
					}

					SkeletonNode& currentBone = skinnedModelComponent.ModelResource->GetSkeletonRootNode();
					recursiveBoneRemapping(currentBone);
				}
			}
			return true;
		}

	private:
		void EntityWalker(Entity entity);
	};
}

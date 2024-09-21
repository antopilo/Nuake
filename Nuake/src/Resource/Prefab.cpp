#include "Prefab.h"
#include "src/Scene/Components/ParentComponent.h"
#include <src/Scene/Components/PrefabComponent.h>
#include <src/Scene/Components/NetScriptComponent.h>
#include <src/Scene/Components/CharacterControllerComponent.h>

namespace Nuake 
{
	Ref<Prefab> Prefab::CreatePrefabFromEntity(Entity entity)
	{
		auto& parentC = entity.GetComponent<ParentComponent>();
		auto& nameComponent = entity.GetComponent<NameComponent>();

		Ref<Prefab> prefab = CreateRef<Prefab>();
		prefab->DisplayName = nameComponent.Name;
		prefab->EntityWalker(entity);
		entity.GetComponent<TransformComponent>() = TransformComponent{};
		prefab->Root = entity;
		return prefab;
	}

	Ref<Prefab> Prefab::New(const std::string& path)
	{
		Ref<Prefab> newPrefab = CreateRef<Prefab>();
		newPrefab->Path = path;

		if (FileSystem::FileExists(path, false))
		{
			std::string prefabTextContent = FileSystem::ReadFile(path);

			if (!prefabTextContent.empty())
			{
				newPrefab->Deserialize(json::parse(prefabTextContent), false);
			}
		}

		return newPrefab;
	}

	Ref<Prefab> Prefab::InstanceInScene(const std::string& path, Scene* scene)
	{
		Ref<Prefab> newPrefab = CreateRef<Prefab>();
		newPrefab->Path = path;

		if (FileSystem::FileExists(path, false))
		{
			std::string prefabTextContent = FileSystem::ReadFile(path);

			if (!prefabTextContent.empty())
			{
				newPrefab->DeserializeIntoScene(json::parse(prefabTextContent), scene);
			}
		}

		return newPrefab;
	}

	Ref<Prefab> Prefab::InstanceOntoRoot(Entity entity, const std::string & path)
	{
		Ref<Prefab> newPrefab = CreateRef<Prefab>();
		newPrefab->Path = path;

		if (FileSystem::FileExists(path, false))
		{
			std::string prefabTextContent = FileSystem::ReadFile(path);

			if (!prefabTextContent.empty())
			{
				json j = json::parse(prefabTextContent);

				if (j == "")
					return newPrefab;

				newPrefab->Root = entity;
				if (j.contains("Entities"))
				{
					std::map<uint32_t, uint32_t> newIdsLut;
					newIdsLut[j["Root"]] = entity.GetID();

					for (json e : j["Entities"])
					{
						if (e["NameComponent"]["ID"] == j["Root"])
						{
							continue; // skip root
						}

						Entity newEntity = { entity.GetScene()->m_Registry.create(), entity.GetScene() };
						newEntity.Deserialize(e); // Id gets overriden by serialized id.

						ParentComponent& parentComponent = newEntity.GetComponent<ParentComponent>();
						if (parentComponent.HasParent && parentComponent.ParentID != j["Root"])
						{
							//auto pId = newEntity.GetComponent<ParentComponent>().ParentID;
							//newEntity.GetComponent<ParentComponent>().ParentID = newIdsLut[pId];
							//auto nc = entity.GetScene()->GetEntityByID(pId).GetComponent<ParentComponent>().RemoveChildren(newEntity);
							//newEntity.GetComponent<ParentComponent>().Parent = entity.GetScene()->GetEntityByID(newIdsLut[pId]);
							//entity.GetScene()->GetEntityByID(newIdsLut[pId]).AddChild(newEntity);
						}
						else if (newEntity.GetComponent<ParentComponent>().ParentID == j["Root"])
						{
							//entity.AddChild(newEntity);
						}

						auto& nameComponent = newEntity.GetComponent<NameComponent>();
						uint32_t oldId = nameComponent.ID;
						uint32_t newId = OS::GetTime();
						nameComponent.Name = nameComponent.Name;
						nameComponent.ID = newId;

						newIdsLut[oldId] = newId;

						newPrefab->AddEntity(newEntity);
					}

					// Set reference to the parent entity to children
					for (auto& e : newPrefab->Entities)
					{
						//auto& parentC = e.GetComponent<ParentComponent>();
						//auto parent = entity.GetScene()->GetEntityByID(newIdsLut[parentC.ParentID]);
						//
						//Logger::Log(parent.GetComponent<NameComponent>().Name);
						//if (parentC.ParentID == j["Root"])
						//{
						//	entity.AddChild(e);
						//}
						//else
						//{
						//	parent.AddChild(e);
						//}
					}

					// Since the bones point to an entity, and we are instancing a prefab, the new skeleton is gonna be pointing to the wrong
					// bones, we need to remap the skeleton to the new entities. We are simply reussing the same map we are using for the 
					// reparenting. Pretty neat.
					std::function<void(SkeletonNode&)> recursiveBoneRemapping = [&recursiveBoneRemapping, &newIdsLut](SkeletonNode& currentBone)
						{
							currentBone.EntityHandle = newIdsLut[currentBone.EntityHandle];
							for (SkeletonNode& bone : currentBone.Children)
							{
								recursiveBoneRemapping(bone);
							}
						};

					// Do the remapping of the skeleton
					for (auto& e : newPrefab->Entities)
					{
						if (!e.HasComponent<SkinnedModelComponent>())
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
			}
		}

		return newPrefab;
	}

	void Prefab::ReInstance()
	{
		if (FileSystem::FileExists(Path, false))
		{
			for (auto& c : Entities)
			{
				if (c.IsValid() && c != Root)
				{
					
				}
			}

			std::string prefabTextContent = FileSystem::ReadFile(Path);
			if (!prefabTextContent.empty())
			{
				Deserialize(json::parse(prefabTextContent), true);
			}
		}
	}

	bool Prefab::Deserialize(const json & j, bool skipRoot)
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
			auto scene = Engine::GetCurrentScene();

			std::map<uint32_t, uint32_t> newIdsLut;
			for (json e : j["Entities"])
			{
				if (e["NameComponent"]["ID"] == j["Root"] && skipRoot)
				{
					newIdsLut[e["NameComponent"]["ID"]] = e["NameComponent"]["ID"];
					continue;
				}

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
					entity.AddComponent<PrefabComponent>();
				}
				else
				{
					entity.AddComponent<PrefabMember>();
				}

				AddEntity(entity);
			}

			// Set reference to the parent entity to children
			for (auto& e : Entities)
			{
				if (!e.IsValid())
				{
					continue;
				}

				if (e.GetID() == Root.GetID())
				{
					continue;
				}

				auto& parentC = e.GetComponent<ParentComponent>();
				auto parent = Engine::GetCurrentScene()->GetEntityByID(newIdsLut[parentC.ParentID]);
				if (parent.GetHandle() == 0)
				{
					Root.AddChild(e);
				}
				else
				{
					parent.AddChild(e);
				}

				e.GetComponent<PrefabMember>().owner = Root.GetID();
			}

			// Since the bones point to an entity, and we are instancing a prefab, the new skeleton is gonna be pointing to the wrong
			// bones, we need to remap the skeleton to the new entities. We are simply reussing the same map we are using for the 
			// reparenting. Pretty neat.
			std::function<void(SkeletonNode&)> recursiveBoneRemapping = [&recursiveBoneRemapping, &newIdsLut](SkeletonNode& currentBone)
				{
					currentBone.EntityHandle = newIdsLut[currentBone.EntityHandle];
					for (SkeletonNode& bone : currentBone.Children)
					{
						recursiveBoneRemapping(bone);
					}
				};

			// Do the remapping of the skeleton
			for (auto& e : Entities)
			{
				if (!e.IsValid())
				{
					continue;
				}

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

	void Prefab::EntityWalker(Entity entity)
	{
		Root = entity;
		entity.GetComponent<NameComponent>().IsPrefab = true;
		Entities.push_back(entity);

		for (const Entity& e : entity.GetComponent<ParentComponent>().Children)
		{
			EntityWalker(e);
		}
	}
}
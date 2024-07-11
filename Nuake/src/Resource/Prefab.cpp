#include "Prefab.h"
#include "src/Scene/Components/ParentComponent.h"

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
				newPrefab->Deserialize(json::parse(prefabTextContent));
			}
		}

		return newPrefab;
	}

	Ref<Prefab> Prefab::InstanceInScene(const std::string& path, Ref<Scene> scene)
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
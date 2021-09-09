#include "Prefab.h"
#include "src/Scene/Components/ParentComponent.h"

namespace Nuake {
	Ref<Prefab> Prefab::CreatePrefabFromEntity(Entity entity)
	{
		Ref<Prefab> prefab = CreateRef<Prefab>();
		ParentComponent parentC = entity.GetComponent<ParentComponent>();
		prefab->EntityWalker(entity);

		return prefab;
	}

	Ref<Prefab> Prefab::New(const std::string& path)
	{
		Ref<Prefab> newPrefab = CreateRef<Prefab>();
		newPrefab->Path = path;

		std::string prefabTextContent = FileSystem::ReadFile(path);
		newPrefab->Deserialize(prefabTextContent);

		return newPrefab;
	}

	void Prefab::EntityWalker(Entity entity)
	{
		entity.GetComponent<NameComponent>().IsPrefab = true;
		Entities.push_back(entity);

		for (const Entity& e : entity.GetComponent<ParentComponent>().Children)
		{
			EntityWalker(e);
		}
	}
}
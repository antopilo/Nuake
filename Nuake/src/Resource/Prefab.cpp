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
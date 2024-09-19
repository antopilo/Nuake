#pragma once
#include "src/Core/Core.h"
#include "src/Resource/Prefab.h"

namespace Nuake {
	class PrefabComponent
	{
	public:
		Ref<Prefab> PrefabInstance;

		void SetPrefab(Ref<Prefab> prefab)
		{
			PrefabInstance = prefab;
		}
	};

	class PrefabMember
	{
	public:
		Entity owner;
	};
}
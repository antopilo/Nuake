#pragma once
#include "src/Core/Core.h"
#include "src/Resource/Prefab.h"

namespace Nuake {
	class PrefabComponent
	{
	public:
		Ref<Prefab> PrefabInstance;
        std::string Path;

        bool isInitialized = false;

		void SetPrefab(Ref<Prefab> prefab)
		{
			PrefabInstance = prefab;
            Path = prefab->Path;
		}

        json Serialize()
        {
            BEGIN_SERIALIZE();
            SERIALIZE_VAL(Path);
            END_SERIALIZE();
        }

        bool Deserialize(const json& j)
        {
            DESERIALIZE_VAL(Path);
            return true;
        }

        void PostDeserialize(Scene& scene)
        {
            //PrefabInstance->ReInstance();
            //SetPrefab(Prefab::InstanceInScene(Path, &scene));
            //PrefabInstance->Root.GetComponent<NameComponent>().IsPrefab = true;
            //PrefabInstance->Root.AddComponent<PrefabComponent>();
        }
	};

	class PrefabMember
	{
	public:
		uint32_t owner = 0;

        json Serialize()
        {
            BEGIN_SERIALIZE();
            SERIALIZE_VAL(owner);
            END_SERIALIZE();
        }

        bool Deserialize(const json& j)
        {
            owner = j["owner"];
            return true;
        }
	};
}
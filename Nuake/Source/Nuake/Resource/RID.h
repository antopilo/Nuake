#pragma once
#include "Nuake/Core/Core.h"
#include "Nuake/Resource/UUID.h"

#include "Nuake/Resource/ResourceManager.h"

namespace Nuake
{
	class Resource;

	class RID
	{
	public:
		UUID ID;
		Ref<Resource> Data;

		RID() : ID(0), Data(nullptr) {}
		RID(UUID id) : ID(id) {}
		
	public:
		template<typename T>
		Ref<T> Get()
		{
			if (!Data)
			{
				Load<T>();
			}

			return std::static_pointer_cast<T>(Data);
		}

		template<typename T>
		void Load()
		{
			if (!Data)
			{
				Data = ResourceManager::GetResource<T>(ID);
			}
		}
	};
}
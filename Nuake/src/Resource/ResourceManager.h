#pragma once
#include "src/Core/Core.h"
#include "src/Resource/Resource.h"
#include "src/Resource/UUID.h"

#include <map>

namespace Nuake 
{
	class ResourceManager 
	{
	private:
		static std::map<UUID, Ref<Resource>> m_Resources;

	public:
		static Ref<Resource> GetResource(const UUID& uuid);

		static void RegisterResource(Ref<Resource> resource)
		{
			m_Resources[resource->ID] = resource;
		}

		static bool IsResourceLoaded(const UUID& uuid)
		{
			return m_Resources.find(uuid) != m_Resources.end();
		}

		template<typename R>
		static Ref<R> GetResource(const UUID& uuid)
		{
			return std::static_pointer_cast<R>(m_Resources[uuid]);
		}
	};
}
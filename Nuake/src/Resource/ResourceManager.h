#pragma once
#include "src/Core/Core.h"
#include "src/Resource/Resource.h"
#include "src/Resource/UUID.h"

#include <map>

namespace Nuake {
	class ResourceManager {
	private:
		static std::map<UUID, Ref<Resource>> m_Resources;

	public:
		static Ref<Resource> GetResource(const UUID& uuid);

		template<typename R>
		static Ref<R> LoadResource(const std::string& path);

		void RegisterResource(Ref<Resource> resource)
		{
			m_Resources[resource->Id] = resource;
		}

		template<typename R>
		static Ref<R> GetResource(const UUID& uuid)
		{
			return reinterpret_pointer_cast<Ref<R>>(m_Resources[uuid]);
		}
	};
}
#pragma once
#include "src/Core/Core.h"
#include "src/Resource/Resource.h"
#include "src/Resource/UUID.h"

#include <map>

namespace Nuake {
	class ResourceManager {
	private:
		static std::unordered_map<UUID, Ref<Resource>> m_Resources;

	public:
		static Ref<Resource> GetResource(const UUID& uuid);
	};
}
#pragma once
#include "Nuake/Core/Core.h"
#include "Nuake/Rendering/Materials/Material.h"

#include <vector>

namespace Nuake {
	class MaterialCollection
	{

	private:
		std::vector<Ref<Material>> m_Materials;
	public:

		std::string Path;
		MaterialCollection(const std::string path)
		{

		}
	};
}
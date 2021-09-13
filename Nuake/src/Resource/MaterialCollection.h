#pragma once
#include "src/Core/Core.h"
#include "src/Rendering/Materials/Material.h"

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
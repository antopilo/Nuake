#pragma once
#include <vector>
#include <map>

#include "src/Core/Core.h"
#include "src/Rendering/Textures/Material.h"
#include "src/Rendering/Mesh/Mesh.h"

class RenderList
{
private:
	std::map<Ref<Material>, std::vector<Ref<Mesh>>> m_RenderList;

public:

	RenderList() {
		this->m_RenderList = std::map<Ref<Material>, std::vector<Ref<Mesh>>>();
	}

	void AddToRenderList(Ref<Mesh> mesh)
	{

	}
};
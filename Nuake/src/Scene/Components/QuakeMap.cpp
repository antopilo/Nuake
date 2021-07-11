#include "QuakeMap.h"
#include "src/Core/Core.h"
#include "src/Core/MaterialManager.h"

namespace Nuake {
	void QuakeMapComponent::Draw()
	{
		for (auto m : m_Meshes) 
			m->Draw();
	}

	void QuakeMapComponent::Load(std::string path, bool collisions)
	{
		if (Path == path)
			return;

		Path = path;
	}


	void QuakeMapComponent::DrawEditor()
	{

	}
}
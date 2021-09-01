#include "QuakeMap.h"

namespace Nuake {
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
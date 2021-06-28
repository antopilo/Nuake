#include "QuakeMap.h"
#include "../Core/Core.h"
#include "../Core/MaterialManager.h"


void QuakeMapComponent::Draw()
{
    for (auto m : m_Meshes) {
        m->Draw();
    }
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
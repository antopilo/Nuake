#include "TrenchbroomSystem.h"

#include "Nuake/Scene/Scene.h"
#include "Nuake/Scene/Components/QuakeMap.h"

namespace Nuake
{
	TrenchbroomSystem::TrenchbroomSystem(Scene* scene)
	{
		m_Scene = scene;
	}

	bool TrenchbroomSystem::Init()
	{
		return true;
	}

	void TrenchbroomSystem::Update(Timestep ts)
	{

	}

	void TrenchbroomSystem::FixedUpdate(Timestep ts)
	{

	}

	void TrenchbroomSystem::EditorUpdate()
	{
		auto quakeMapView = m_Scene->m_Registry.view<QuakeMapComponent>();
		for (auto e : quakeMapView)
		{
			const auto& quakeMapComponent = quakeMapView.get<QuakeMapComponent>(e);

		}
	}

	void TrenchbroomSystem::Exit()
	{

	}
}

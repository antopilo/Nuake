#include "ParticleSystem.h"

#include "src/Scene/Scene.h"
#include "src/Scene/Components/QuakeMap.h"

namespace Nuake
{
	ParticleSystem::ParticleSystem(Scene* scene)
	{
		m_Scene = scene;
	}

	bool ParticleSystem::Init()
	{
		return true;
	}

	void ParticleSystem::Update(Timestep ts)
	{

	}

	void ParticleSystem::FixedUpdate(Timestep ts)
	{

	}

	void ParticleSystem::EditorUpdate()
	{

	}

	void ParticleSystem::Exit()
	{

	}
}

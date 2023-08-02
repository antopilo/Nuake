#include "ParticleSystem.h"

#include "src/Scene/Scene.h"
#include "src/Scene/Entities/Entity.h"
#include "src/Scene/Components/ParticleEmitterComponent.h"


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
		auto& view = m_Scene->m_Registry.view<TransformComponent, ParticleEmitterComponent>();
		for (auto& e : view)
		{
			auto [transformComponent, emitterComponent] = view.get<TransformComponent, ParticleEmitterComponent>(e);
			Entity ent = Entity({ e, m_Scene });

			// Copy emitter settings from component to real emitter
			auto& emitter = emitterComponent.Emitter;
			emitter.Amount = emitterComponent.Amount;
			emitter.Gravity = emitterComponent.Gravity;
			emitter.GravityRandom = emitterComponent.GravityRandom;
			emitter.Radius = emitterComponent.Radius;
			emitter.Life = emitterComponent.Life;

			// Spawn particle if possible
			emitterComponent.Emitter.SpawnParticle();
		}
	}

	void ParticleSystem::FixedUpdate(Timestep ts)
	{
		auto& view = m_Scene->m_Registry.view<TransformComponent, ParticleEmitterComponent>();
		for (auto& e : view)
		{
			auto [transformComponent, emitterComponent] = view.get<TransformComponent, ParticleEmitterComponent>(e);
			Entity ent = Entity({ e, m_Scene });

			emitterComponent.Emitter.Update(ts);
		}
	}

	void ParticleSystem::EditorUpdate()
	{

	}

	void ParticleSystem::Exit()
	{

	}
}

#pragma once
#include "Particle.h"

#include "src/Core/Core.h"
#include "src/Core/Maths.h"
#include "src/Core/Timestep.h"

#include <random>

namespace Nuake
{
	class ParticleEmitter
	{
	public:
		float Amount = 0.0f;
		float Life = 5.0f;
		Vector3 Gravity = Vector3(0, 0, 0);
		float GravityRandom = 0.0f;
		float Radius = 1.0f;

	private:
		std::mt19937 m_MT;
		std::uniform_real_distribution<float> m_Random;

	public:
		ParticleEmitter();
		~ParticleEmitter() = default;

		void SpawnParticle();
		void Update(Timestep ts);

		std::vector<Particle> Particles;
	};
}
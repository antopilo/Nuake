#pragma once
#include "Particle.h"

#include "src/Core/Core.h"
#include "src/Core/Maths.h"
#include "src/Core/Timestep.h"

#include "src/Rendering/Textures/Material.h"

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
		float Rate = 0.0f;
		Color ParticleColor;

		float LifeRandom = 0.0f;
		float ScaleRandom = 0.0f;

		bool IsGlobalSpace = false;
		Vector3 GlobalOrigin;

	private:
		float RateTimer = 0.0f;
		std::mt19937 m_MT;
		std::uniform_real_distribution<float> m_Random;
		std::uniform_real_distribution<float> m_ScaleRandom;
		std::uniform_real_distribution<float> m_LifeRandom;
		void RecreateRandom();

	public:
		ParticleEmitter();
		~ParticleEmitter() = default;

		void SetGlobalOrigin(const Vector3& origin);

		void SpawnParticle();
		void Update(Timestep ts);

		void SetRadius(float radius);
		void SetScaleRandom(float scaleRandom);
		void SetLifeRandom(float lifeRandom);

		std::vector<Particle> Particles;
	};
}
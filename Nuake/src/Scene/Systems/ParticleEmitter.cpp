#include "ParticleEmitter.h"

namespace Nuake
{
	ParticleEmitter::ParticleEmitter()
	{
		m_MT = std::mt19937(std::random_device()());
		SetRadius(1.0f);
	}

	void ParticleEmitter::RecreateRandom()
	{

	}

	void ParticleEmitter::SetGlobalOrigin(const Vector3& origin)
	{
		GlobalOrigin = origin;
	}

	void ParticleEmitter::SpawnParticle()
	{
		const bool canSpawnParticle = Particles.size() < Amount;
		if (!canSpawnParticle)
		{
			return;
		}

		if (Rate == 0.0f || RateTimer > Rate)
		{
			auto initialPosition = Vector3(m_Random(m_MT), m_Random(m_MT), m_Random(m_MT));
			const auto initialVelocity = Vector3();
			const auto initialColor = ParticleColor;
			const float initialLife = Life;

			if (IsGlobalSpace)
			{
				initialPosition = initialPosition + GlobalOrigin;
			}

			Particles.push_back({
				initialPosition,
				initialVelocity,
				initialColor,
				initialLife
			});

			RateTimer = 0.0f;
		}
	}

	void ParticleEmitter::Update(Timestep ts)
	{
		if (Rate != 0.0f)
		{
			RateTimer += ts;
		}

		std::vector<uint32_t> deletionQueue;
		int i = 0;
		for (auto& p : Particles)
		{
			p.Life -= ts;

			if (p.Life <= 0.0f) // The particle has died.
			{
				deletionQueue.push_back(i);
			}

			i++;
		}

		// Delete all dead particles.
		int shiftOffset = 0;
		for (int d = deletionQueue.size() - 1; d > 0; d--)
		{
			// Erase shifts the elements
			const auto it = d;
			Particles.erase(Particles.begin() + it);
		}

		for (auto& p : Particles)
		{
			p.Velocity += Gravity * static_cast<float>(ts);
			p.Position += p.Velocity * static_cast<float>(ts);
		}
	}

	void ParticleEmitter::SetRadius(float radius)
	{
		if (Radius != radius)
		{
			Radius = radius;
			m_Random = std::uniform_real_distribution<float>(-Radius, Radius);
		}
	}
}
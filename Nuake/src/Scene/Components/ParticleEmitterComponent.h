#pragma once
#include "src/Core/Core.h"
#include "src/Core/Maths.h"
#include "src/Resource/Serializable.h"

#include "src/Scene/Systems/ParticleEmitter.h"


namespace Nuake
{
	class ParticleEmitterComponent
	{
	public:
		ParticleEmitterComponent() = default;
		~ParticleEmitterComponent() = default;

		Ref<Material> ParticleMaterial = CreateRef<Material>();

		float Amount;

		float LifeRandomness = 0.0f;
		float Life = 1.0f;
		float Rate = 0.0f;

		float ScaleRandomness = 0.0f;
		Vector3 ParticleScale = Vector3(0.1, 0.1, 0.1);

		bool GlobalSpace = false;

		Vector3 Gravity;
		float GravityRandom;

		// For now use a radius, later should use shape.
		float Radius;

		ParticleEmitter Emitter;

	public:
		json Serialize();
		bool Deserialize(const json& j);
	};
}
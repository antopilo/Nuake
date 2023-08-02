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

		Color ParticleColor;
		float Amount;
		float Life;

		Vector3 Gravity;
		float GravityRandom;

		// For now use a radius, later should use shape.
		float Radius;

		ParticleEmitter Emitter;

	public:
		json Serialize();
		bool Deserialize(const std::string& str);
	};
}
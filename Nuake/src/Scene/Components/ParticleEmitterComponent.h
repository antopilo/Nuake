#pragma once
#include "src/Core/Core.h"
#include "src/Resource/Serializable.h"
#include <src/Core/Maths.h>

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

	public:
		json Serialize();
		bool Deserialize(const std::string& str);
	};
}